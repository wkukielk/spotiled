mod dom_color;
mod led_socket;
mod player_event_listener;
mod song_pipeline;
mod spotify;

use anyhow::Result;
use led_socket::WsLedSocketConnector;
use song_pipeline::SongPipeline;

use log::LevelFilter;

use spotify::Spotify;
use tokio::net::TcpListener;

const LED_SOCKET_URL: &str = "ws://TODO-fill-in-device-address";
const CONTROL_SERVER_ADDR: &str = "TODO-fill-in-control-server-addr";

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

    let player = spotify.player();
    let connector = WsLedSocketConnector {
        url: LED_SOCKET_URL.to_string(),
    };
    let mut pipeline = SongPipeline::new(connector, move || player.get_player_event_channel());

    rt.block_on(async {
        let listener = TcpListener::bind(CONTROL_SERVER_ADDR).await?;
        pipeline.serve(listener).await
    })
}
