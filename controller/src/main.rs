mod config;
mod dom_color;
mod led_socket;
mod song_pipeline;
mod spotify;

use anyhow::Result;
use led_socket::LedSocketConnector;
use led_socket::WsLedSocket;
use song_pipeline::SongPipeline;

use log::LevelFilter;

use spotify::ISpotify;
use spotify::Spotify;
use std::error::Error;
use std::sync::Arc;
use tokio::net::TcpListener;

const LED_SOCKET_URL: &str = "ws://spotiled-esp32.local:8000/ws";

fn create_tokio_runtime() -> tokio::runtime::Runtime {
    tokio::runtime::Builder::new_multi_thread()
        .enable_all()
        .build()
        .expect("Failed to build Tokio runtime")
}

fn main() -> Result<(), Box<dyn Error>> {
    env_logger::builder()
        .filter_level(LevelFilter::Info)
        .filter_module("librespot", LevelFilter::Debug)
        .init();

    let cfg = config::load("config.toml".as_ref())?;

    let rt = create_tokio_runtime();
    let mut spotify = Spotify::new();
    spotify.init(&rt)?;
    let spotify: Arc<dyn ISpotify> = Arc::new(spotify);

    type Connector = LedSocketConnector<WsLedSocket>;
    let connector = Connector::new(LED_SOCKET_URL.to_string());
    let mut pipeline = SongPipeline::new(spotify, connector);

    rt.block_on(async {
        let listener = TcpListener::bind(&cfg.bind_address).await?;
        pipeline.serve(listener).await
    })?;

    Ok(())
}
