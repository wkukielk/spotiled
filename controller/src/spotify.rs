pub mod playback_event_listener;

use anyhow::Result;
use librespot::core::Error as LibrespotError;
use librespot::{
    core::{
        SpotifyUri,
        authentication::Credentials,
        cache::Cache,
        config::SessionConfig,
        dealer::{manager::BoxedStreamResult, protocol::Message},
        session::Session,
    },
    metadata::audio::AudioItem,
};
use thiserror::Error;

#[derive(Error, Debug)]
pub enum SpotifyError {
    #[error("Need to init Spotify before")]
    NotInit,

    #[error("Spotify internal error: {0}")]
    Librespot(#[from] LibrespotError),
}

const CACHE_PATH: &str = ".cache";
const CACHE_PATH_AUDIO: &str = ".cache/files";

#[cfg_attr(test, mockall::automock)]
trait OAuthCredentialsProvider {
    fn fetch(&self, client_id: &str) -> Result<Credentials>;
}

struct LibrespotOAuthProvider;

impl OAuthCredentialsProvider for LibrespotOAuthProvider {
    fn fetch(&self, client_id: &str) -> Result<Credentials> {
        let token = librespot::oauth::OAuthClientBuilder::new(
            client_id,
            "http://127.0.0.1:8898/login",
            vec!["streaming"],
        )
        .open_in_browser()
        .build()?
        .get_access_token()?;

        Ok(Credentials::with_access_token(token.access_token))
    }
}

pub struct Spotify {
    cache: Option<Cache>,
    session: Option<Session>,
}

impl Spotify {
    pub fn new() -> Self {
        Spotify {
            cache: Spotify::create_cache(),
            session: None,
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
            &LibrespotOAuthProvider,
        )
    }

    async fn connect(
        cache: Option<Cache>,
        credentials: Credentials,
    ) -> Result<Session, SpotifyError> {
        let session = Session::new(SessionConfig::default(), cache);
        session.connect(credentials, true).await?;
        session.dealer().start().await?;
        Ok(session)
    }

    pub fn init(&mut self, rt: &tokio::runtime::Runtime) -> Result<()> {
        let credentials = self.get_credentials()?;
        let session = rt.block_on(Spotify::connect(self.cache.clone(), credentials))?;
        self.session = Some(session);
        Ok(())
    }

    pub fn get_playback_event_stream(&self) -> Result<BoxedStreamResult<Message>, SpotifyError> {
        let session = self.session.as_ref().ok_or(SpotifyError::NotInit)?;
        let stream = session.dealer().listen_for(
            "hm://herodotus/uri/spotify:list:play-history:v1/resume-point-revision",
            |msg| Ok(msg),
        )?;
        Ok(stream)
    }

    pub async fn get_audio_item(&self, uri: &str) -> Result<AudioItem, SpotifyError> {
        let session = self.session.as_ref().ok_or(SpotifyError::NotInit)?;
        let spotify_uri = SpotifyUri::from_uri(uri)?;
        Ok(AudioItem::get_file(session, spotify_uri).await?)
    }
}

#[cfg(test)]
mod spotify_tests;
