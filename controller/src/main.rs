mod config;
mod dom_color;
mod led_socket;
mod mdns_service;
mod song_pipeline;
mod spotify;

use anyhow::Result;
use led_socket::WsLedSocket;
use mdns_service::MdnsService;
use song_pipeline::Pipeline;

use log::LevelFilter;

use spotify::ISpotify;
use spotify::Spotify;
use std::error::Error;
use std::sync::Arc;
use tokio::net::TcpListener;

fn create_tokio_runtime() -> tokio::runtime::Runtime {
    tokio::runtime::Builder::new_multi_thread()
        .enable_all()
        .build()
        .expect("Failed to build Tokio runtime")
}

fn get_hostname() -> String {
    let instance_name_local = hostname::get()
        .map(|h| h.to_string_lossy().into_owned())
        .unwrap_or_else(|_| "spotiled".to_string());
    instance_name_local.split(".").next().unwrap().to_string()
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

    let mut pipeline = Pipeline::<WsLedSocket>::new(spotify);

    let instance_name = get_hostname();
    log::info!("{}", instance_name);

    rt.block_on(async {
        let listener = TcpListener::bind(&cfg.bind_address).await?;
        let port = listener.local_addr()?.port();
        let _mdns = MdnsService::new("_ws._tcp", &instance_name, port)?;

        pipeline.serve(listener).await
    })?;

    Ok(())
}
