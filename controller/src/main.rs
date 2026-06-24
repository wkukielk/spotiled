mod dom_color;
mod led_socket;
mod player_event_listener;
mod song_pipeline;
mod spotify;

use anyhow::Result;
use led_socket::WsLedSocket;
use song_pipeline::SongPipeline;

use log::LevelFilter;

use spotify::Spotify;

const LED_SOCKET_URL: &str = "ws://TODO-fill-in-device-address";

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

    let socket = rt.block_on(WsLedSocket::connect(LED_SOCKET_URL))?;
    let mut pipeline = SongPipeline::new(spotify.get_player_event_channel(), socket);
    rt.block_on(pipeline.run());
    Ok(())
}
