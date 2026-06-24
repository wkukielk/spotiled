mod dom_color;
mod player_event_listener;
mod song_pipeline;
mod spotify;

use anyhow::Result;
use player_event_listener::PlayerEventListener;
use song_pipeline::SongPipeline;

use log::LevelFilter;

use spotify::Spotify;

fn create_tokio_runtime() -> tokio::runtime::Runtime {
    tokio::runtime::Builder::new_multi_thread()
        .enable_all()
        .build()
        .expect("Failed to build Tokio runtime")
}

fn main() -> Result<()> {
    env_logger::builder()
        .filter_module("librespot", LevelFilter::Debug)
        .init();

    let rt = create_tokio_runtime();
    let mut spotify = Spotify::new();
    spotify.init(&rt)?;

    let listener =
        PlayerEventListener::new(spotify.get_player_event_channel(), SongPipeline::new());
    rt.block_on(listener.run());
    Ok(())
}
