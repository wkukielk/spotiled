mod dom_color;
mod player_event_listener;
mod song_pipeline;

use librespot::{
    connect::{ConnectConfig, LoadRequest, LoadRequestOptions, Spirc},
    core::{
        Error, authentication::Credentials, cache::Cache, config::SessionConfig, session::Session,
    },
    playback::{
        config::{PlayerConfig},
        mixer,
        mixer::MixerConfig,
        player::Player,
    },
};
use player_event_listener::{
    PlayerEventListener,
    MySink
};
use song_pipeline::SongPipeline;

use log::LevelFilter;

const CACHE: &str = ".cache";
const CACHE_FILES: &str = ".cache/files";

fn main() -> Result<(), Error> {
    env_logger::builder()
        .filter_module("librespot", LevelFilter::Debug)
        .init();

    let session_config = SessionConfig::default();
    let player_config = PlayerConfig::default();
    let connect_config = ConnectConfig::default();
    let mixer_config = MixerConfig::default();
    let request_options = LoadRequestOptions::default();

    let mixer_builder = mixer::find(None).unwrap();

    let cache = Cache::new(Some(CACHE), Some(CACHE), Some(CACHE_FILES), None)?;
    let credentials = cache
        .credentials()
        .ok_or(Error::unavailable("credentials not cached"))
        .or_else(|_| {
            librespot::oauth::OAuthClientBuilder::new(
                &session_config.client_id,
                "http://127.0.0.1:8898/login",
                vec!["streaming"],
            )
            .open_in_browser()
            .build()?
            .get_access_token()
            .map(|t| Credentials::with_access_token(t.access_token))
        })?;

    let rt = tokio::runtime::Builder::new_multi_thread()
        .enable_all()
        .build()
        .expect("Failed to build Tokio runtime");

    rt.block_on(async move {
        let session = Session::new(session_config, Some(cache));
        let mixer = mixer_builder(mixer_config)?;
        let player = Player::new(
            player_config,
            session.clone(),
            mixer.get_soft_volume(),
            move || Box::new(MySink::new()),
        );

        let listener =
            PlayerEventListener::new(player.get_player_event_channel(), SongPipeline::new());
        tokio::spawn(listener.run());

        let (spirc, spirc_task) =
            Spirc::new(connect_config, session.clone(), credentials, player, mixer).await?;

        spirc.activate()?;
        spirc.load(LoadRequest::from_context_uri(
            format!("spotify:user:{}:collection", session.username()),
            request_options,
        ))?;
        spirc.play()?;

        spirc_task.await;
        Ok(())
    })
}
