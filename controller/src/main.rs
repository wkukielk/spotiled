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
use std::error::Error;
use tokio::net::TcpListener;

fn get_mdns_hostname() -> Result<String, Box<dyn Error>> {
    let os_hostname = hostname::get()?;

    let hostname_str = os_hostname
        .into_string()
        .map_err(|_| "Failed to convert hostname from OsString to String")?;

    let mdns_hostname = format!("{}.local.", hostname_str.trim_end_matches('.'));

    Ok(mdns_hostname)
}

fn get_service_bind_address() -> Result<String, Box<dyn Error>> {
    let hostname = get_mdns_hostname()?;
    let address = format!("{}:6000", hostname);
    Ok(address)
}

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

    let rt = create_tokio_runtime();
    let mut spotify = Spotify::new();
    spotify.init(&rt)?;

    let player = spotify.player();
    let connector = WsLedSocketConnector {
        url: LED_SOCKET_URL.to_string(),
    };
    let get_channel = move || player.get_player_event_channel();
    let mut pipeline = SongPipeline::new(connector, get_channel);

    let address = get_service_bind_address()?;
    rt.block_on(async {
        let listener = TcpListener::bind(address).await?;
        pipeline.serve(listener).await
    })?;

    Ok(())
}
