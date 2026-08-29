pub mod playback_event_listener;

use std::sync::RwLock;

use anyhow::Result;
use async_trait::async_trait;
use futures_util::StreamExt;
use futures_util::stream::poll_fn;
use librespot::core::Error as LibrespotError;
use librespot::{
    core::{
        SpotifyUri, authentication::Credentials, cache::Cache, config::SessionConfig,
        dealer::manager::BoxedStreamResult, mercury::MercuryResponse, session::Session,
    },
    metadata::audio::AudioItem as LibrespotAudioItem,
    metadata::audio::item::CoverImage,
};
use oauth2::{
    AuthUrl, AuthorizationCode, ClientId, PkceCodeVerifier, RedirectUrl, TokenResponse, TokenUrl,
    basic::BasicClient,
};
use thiserror::Error;

#[derive(Error, Debug)]
pub enum SpotifyError {
    #[error("Spotify internal error: {0}")]
    Librespot(#[from] LibrespotError),

    #[error("Failed to exchange authorization code for access token: {0}")]
    AuthExchange(String),

    #[error("No cache found")]
    NoCache,
}

const CACHE_PATH: &str = ".cache";
const CACHE_PATH_AUDIO: &str = ".cache/files";
const OAUTH_REDIRECT_URI: &str = "http://127.0.0.1:8898/login";
const OAUTH_AUTH_URL: &str = "https://accounts.spotify.com/authorize";
const OAUTH_TOKEN_URL: &str = "https://accounts.spotify.com/api/token";

pub struct AudioItem {
    pub covers: Vec<CoverImage>,
}

#[cfg_attr(test, mockall::automock)]
#[async_trait]
pub trait ISpotify: Send + Sync {
    /// The connected account's canonical Spotify username, or `None` if not connected.
    /// Never attempts to (re)connect — a status check should never itself trigger a network
    /// round-trip.
    fn status(&self) -> Option<String>;
    async fn get_playback_event_stream(
        &self,
    ) -> Result<BoxedStreamResult<MercuryResponse>, SpotifyError>;
    async fn get_audio_item(&self, uri: &str) -> Result<AudioItem, SpotifyError>;
    async fn authorize(&self, auth_code: &str, code_verifier: &str) -> Result<(), SpotifyError>;
}

pub struct Spotify {
    cache: Option<Cache>,
    session: RwLock<Option<Session>>,
}

impl Spotify {
    const HM_URL: &str = "hm://herodotus/uri/spotify:list:play-history:v1/resume-point-revision";

    fn encode_mercury_uri(uri: &str) -> String {
        let mut parts = uri.split('/');
        std::iter::once(parts.next().unwrap_or_default().to_string())
            .chain(
                parts.map(|component| {
                    form_urlencoded::byte_serialize(component.as_bytes()).collect()
                }),
            )
            .collect::<Vec<String>>()
            .join("/")
    }

    pub fn new() -> Self {
        Spotify {
            cache: Spotify::create_cache(),
            session: RwLock::new(None),
        }
    }

    fn create_cache() -> Option<Cache> {
        let size_limit = None;
        let cache = Cache::new(
            Some(CACHE_PATH),
            Some(CACHE_PATH),
            Some(CACHE_PATH_AUDIO),
            size_limit,
        )
        .inspect_err(|err| {
            log::warn!("Failed to initialize Spotify cache: {:?}", err);
        });
        cache.ok()
    }

    fn get_credentials(&self) -> Result<Credentials, SpotifyError> {
        let cached_credentials = self.cache.as_ref().and_then(|c| c.credentials());
        match cached_credentials {
            Some(credentials) => Ok(credentials),
            None => Err(SpotifyError::NoCache)
        }
    }

    async fn connect(
        cache: Option<Cache>,
        credentials: Credentials,
    ) -> Result<Session, SpotifyError> {
        let session = Session::new(SessionConfig::default(), cache);
        let store_credentials = true;
        session.connect(credentials, store_credentials).await?;
        session.dealer().start().await?;
        Ok(session)
    }

    async fn session_or_reconnect(&self) -> Result<Session, SpotifyError> {
        let current = self.session.read().unwrap().clone();
        if let Some(session) = current
            && !session.is_invalid()
        {
            return Ok(session);
        }

        let credentials = self.get_credentials()?;
        let session = Spotify::connect(self.cache.clone(), credentials).await?;
        *self.session.write().unwrap() = Some(session.clone());
        Ok(session)
    }

    async fn exchange_authorization_code(
        client_id: &str,
        auth_code: &str,
        code_verifier: &str,
        token_url: &str,
    ) -> Result<Credentials, SpotifyError> {
        let auth_url =
            AuthUrl::new(OAUTH_AUTH_URL.to_string()).expect("hardcoded auth url is valid");
        let token_url = TokenUrl::new(token_url.to_string())
            .map_err(|e| SpotifyError::AuthExchange(e.to_string()))?;
        let redirect_url = RedirectUrl::new(OAUTH_REDIRECT_URI.to_string())
            .expect("hardcoded redirect uri is valid");

        let client = BasicClient::new(ClientId::new(client_id.to_string()))
            .set_auth_uri(auth_url)
            .set_token_uri(token_url)
            .set_redirect_uri(redirect_url);

        let http_client = reqwest::Client::new();
        let token = client
            .exchange_code(AuthorizationCode::new(auth_code.to_string()))
            .set_pkce_verifier(PkceCodeVerifier::new(code_verifier.to_string()))
            .request_async(&http_client)
            .await
            .map_err(|e| SpotifyError::AuthExchange(e.to_string()))?;

        Ok(Credentials::with_access_token(
            token.access_token().secret().to_string(),
        ))
    }
}

#[async_trait]
impl ISpotify for Spotify {
    fn status(&self) -> Option<String> {
        match self.session.read().unwrap().clone() {
            Some(session) if !session.is_invalid()
                => Some(session.user_data().canonical_username),
            _ => None,
        }
    }

    async fn get_playback_event_stream(
        &self,
    ) -> Result<BoxedStreamResult<MercuryResponse>, SpotifyError> {
        let session = self.session_or_reconnect().await?;
        let uri = Spotify::encode_mercury_uri(Spotify::HM_URL);
        let mut receiver = session.mercury().listen_for(uri).await;
        let stream = poll_fn(move |cx| receiver.poll_recv(cx)).map(Ok);
        Ok(Box::pin(stream))
    }

    async fn get_audio_item(&self, uri: &str) -> Result<AudioItem, SpotifyError> {
        let session = self.session_or_reconnect().await?;
        let spotify_uri = SpotifyUri::from_uri(uri)?;
        let audio_item = LibrespotAudioItem::get_file(&session, spotify_uri).await?;
        Ok(AudioItem {
            covers: audio_item.covers,
        })
    }

    async fn authorize(&self, auth_code: &str, code_verifier: &str) -> Result<(), SpotifyError> {
        let client_id = SessionConfig::default().client_id;
        let credentials = Spotify::exchange_authorization_code(
            &client_id,
            auth_code,
            code_verifier,
            OAUTH_TOKEN_URL,
        )
        .await?;
        let session = Spotify::connect(self.cache.clone(), credentials).await?;
        *self.session.write().unwrap() = Some(session);
        Ok(())
    }
}

#[cfg(test)]
mod spotify_tests;

#[cfg(test)]
mod spotify_integration_tests;
