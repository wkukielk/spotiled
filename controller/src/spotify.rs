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
    #[error("Need to init Spotify before")]
    NotInit,

    #[error("Spotify internal error: {0}")]
    Librespot(#[from] LibrespotError),

    #[error("Failed to exchange authorization code for access token: {0}")]
    AuthExchange(String),
}

const CACHE_PATH: &str = ".cache";
const CACHE_PATH_AUDIO: &str = ".cache/files";
// Must match the redirect_uri a frontend uses when it builds the Spotify
// authorization URL itself (mirroring what OAuthClientBuilder does), since
// Spotify validates it against the one used to request the auth code.
const OAUTH_REDIRECT_URI: &str = "http://127.0.0.1:8898/login";
const OAUTH_AUTH_URL: &str = "https://accounts.spotify.com/authorize";
const OAUTH_TOKEN_URL: &str = "https://accounts.spotify.com/api/token";

#[cfg_attr(test, mockall::automock)]
trait OAuthCredentialsProvider {
    fn fetch(&self, client_id: &str) -> Result<Credentials>;
}

struct LibrespotOAuthProvider;

impl OAuthCredentialsProvider for LibrespotOAuthProvider {
    fn fetch(&self, client_id: &str) -> Result<Credentials> {
        let token = librespot::oauth::OAuthClientBuilder::new(
            client_id,
            OAUTH_REDIRECT_URI,
            vec!["streaming"],
        )
        .open_in_browser()
        .build()?
        .get_access_token()?;

        Ok(Credentials::with_access_token(token.access_token))
    }
}

pub struct AudioItem {
    pub covers: Vec<CoverImage>,
}

#[cfg_attr(test, mockall::automock)]
#[async_trait]
pub trait ISpotify: Send + Sync {
    fn init(&mut self, rt: &tokio::runtime::Runtime) -> Result<()>;
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
    // librespot percent-encodes each path segment (except the first) of an
    // incoming Mercury push's URI before matching it against subscriptions
    // (see librespot-core mercury/mod.rs, MercuryManager::complete_request).
    // HM_URL's segments contain literal colons, which aren't in librespot's
    // "safe" character set, so the subscription has to be encoded the same
    // way or the prefix match silently fails.
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

    fn resolve_credentials(
        cached: Option<Credentials>,
        client_id: &str,
        oauth: &impl OAuthCredentialsProvider,
    ) -> Result<Credentials> {
        match cached {
            Some(credentials) => Ok(credentials),
            None => oauth.fetch(client_id),
        }
    }

    fn get_credentials(&self) -> Result<Credentials> {
        let session_config = SessionConfig::default();
        let cached_credentials = self.cache.as_ref().and_then(|c| c.credentials());
        Spotify::resolve_credentials(
            cached_credentials,
            &session_config.client_id,
            &LibrespotOAuthProvider {},
        )
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

    // token_url is a parameter (rather than always OAUTH_TOKEN_URL) so tests
    // can point it at a mock server instead of the real Spotify backend.
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
    fn init(&mut self, rt: &tokio::runtime::Runtime) -> Result<()> {
        let credentials = self.get_credentials()?;
        let session = rt.block_on(Spotify::connect(self.cache.clone(), credentials))?;
        *self.session.write().unwrap() = Some(session);
        Ok(())
    }

    async fn get_playback_event_stream(
        &self,
    ) -> Result<BoxedStreamResult<MercuryResponse>, SpotifyError> {
        let session = self
            .session
            .read()
            .unwrap()
            .clone()
            .ok_or(SpotifyError::NotInit)?;
        let uri = Spotify::encode_mercury_uri(Spotify::HM_URL);
        let mut receiver = session.mercury().listen_for(uri).await;
        let stream = poll_fn(move |cx| receiver.poll_recv(cx)).map(Ok);
        Ok(Box::pin(stream))
    }

    async fn get_audio_item(&self, uri: &str) -> Result<AudioItem, SpotifyError> {
        let session = self
            .session
            .read()
            .unwrap()
            .clone()
            .ok_or(SpotifyError::NotInit)?;
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
