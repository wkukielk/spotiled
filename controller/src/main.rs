mod config;
mod dom_color;
mod led_socket;
mod mdns_service;
mod pairing;
mod song_pipeline;
mod spotify;
mod tls_identity;

use anyhow::Result;
use led_socket::WsLedSocket;
use mdns_service::MdnsService;
use song_pipeline::Pipeline;

use log::LevelFilter;

use spotify::ISpotify;
use spotify::Spotify;
use socket2::{Domain, Socket, Type};
use std::error::Error;
use std::io;
use std::net::SocketAddr;
use std::path::Path;
use std::sync::Arc;
use tokio::net::TcpListener;

fn create_tokio_runtime() -> tokio::runtime::Runtime {
    tokio::runtime::Builder::new_multi_thread()
        .enable_all()
        .build()
        .expect("Failed to build Tokio runtime")
}

fn bind_dual_stack(addr: &str) -> io::Result<std::net::TcpListener> {
    let addr: SocketAddr = addr
        .parse()
        .map_err(|e| io::Error::new(io::ErrorKind::InvalidInput, e))?;
    let socket = Socket::new(Domain::IPV6, Type::STREAM, None)?;
    socket.set_only_v6(false)?;
    socket.set_reuse_address(true)?;
    socket.bind(&addr.into())?;
    socket.listen(128)?;
    socket.set_nonblocking(true)?;
    Ok(socket.into())
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
        .filter_module("libmdns::fsm", LevelFilter::Error)
        .init();

    let cfg = config::load("config.toml".as_ref())?;

    let rt = create_tokio_runtime();
    let spotify = Spotify::new();
    let spotify: Arc<dyn ISpotify> = Arc::new(spotify);

    let mut pipeline = Pipeline::<WsLedSocket>::new(spotify);

    let instance_name = get_hostname();
    log::info!("{}", instance_name);

    let (tls_acceptor, fingerprint) = tls_identity::load_or_generate(Path::new("data"), &instance_name)?;

    rt.block_on(async {
        let listener = TcpListener::from_std(bind_dual_stack(&cfg.bind_address)?)?;
        let port = listener.local_addr()?.port();
        let _mdns = MdnsService::new("_s-led-controler._tcp", &instance_name, port)?;

        pipeline.serve_tls(listener, tls_acceptor, fingerprint).await
    })?;

    Ok(())
}
