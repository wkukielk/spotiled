use anyhow::Result;
use librespot::{
    connect::{ConnectConfig, LoadRequest, LoadRequestOptions, Spirc},
    core::{
        Error, authentication::Credentials, cache::Cache, config::SessionConfig, session::Session,
    },
    playback::{
        audio_backend,
        config::PlayerConfig,
        convert::Converter,
        decoder::AudioPacket,
        mixer,
        mixer::MixerConfig,
        player::{Player, PlayerEventChannel},
    },
};
use std::sync::Arc;

struct MySink;

impl MySink {
    fn new() -> Self {
        MySink
    }
}

impl audio_backend::Sink for MySink {
    fn write(&mut self, _: AudioPacket, _: &mut Converter) -> audio_backend::SinkResult<()> {
        Ok(())
    }
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
    player: Option<Arc<Player>>,
}

impl Spotify {
    pub fn new() -> Self {
        Spotify {
            cache: Spotify::create_cache(),
            player: None,
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
        oauth: &dyn OAuthCredentialsProvider,
    ) -> Result<Credentials> {
        match cached {
            Some(credentials) => Ok(credentials),
            None => oauth.fetch(client_id),
        }
    }

    pub fn init(&mut self, rt: &tokio::runtime::Runtime) -> Result<()> {
        let session_config = SessionConfig::default();
        let player_config = PlayerConfig::default();
        let connect_config = ConnectConfig::default();
        let mixer_config = MixerConfig::default();
        let request_options = LoadRequestOptions::default();

        let mixer_builder = mixer::find(None).unwrap();

        let cached_credentials = self.cache.as_ref().and_then(|c| c.credentials());
        let credentials = Spotify::resolve_credentials(
            cached_credentials,
            &session_config.client_id,
            &LibrespotOAuthProvider,
        )?;

        let cache = self.cache.clone();
        let (player, spirc_task) = rt.block_on(async move {
            let session = Session::new(session_config, cache);
            let mixer = mixer_builder(mixer_config)?;
            let player = Player::new(
                player_config,
                session.clone(),
                mixer.get_soft_volume(),
                || Box::new(MySink::new()),
            );

            let (spirc, spirc_task) = Spirc::new(
                connect_config,
                session.clone(),
                credentials,
                player.clone(),
                mixer,
            )
            .await?;

            spirc.activate()?;
            spirc.load(LoadRequest::from_context_uri(
                format!("spotify:user:{}:collection", session.username()),
                request_options,
            ))?;
            spirc.play()?;

            Ok::<_, Error>((player, spirc_task))
        })?;

        self.player = Some(player);
        rt.spawn(spirc_task);
        Ok(())
    }

    pub fn get_player_event_channel(&self) -> PlayerEventChannel {
        self.player
            .as_ref()
            .expect("Spotify::init must be called before get_player_event_channel")
            .get_player_event_channel()
    }
}

#[cfg(test)]
mod tests;
