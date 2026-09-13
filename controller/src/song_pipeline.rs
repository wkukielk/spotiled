mod command_processor;
mod pipeline_controller;
mod song_handler;

use self::command_processor::CommandProcessor;
use self::pipeline_controller::PipelineController;
use anyhow::Result;
use async_trait::async_trait;
use std::fmt::Debug;
use std::sync::Arc;

use futures_util::{SinkExt, StreamExt};
use tokio::io::{AsyncRead, AsyncWrite};
use tokio::net::{TcpListener, TcpStream};
use tokio::sync::mpsc;
use tokio_rustls::TlsAcceptor;
use tokio_rustls::server::TlsStream;
use tokio_tungstenite::WebSocketStream;
use tokio_tungstenite::tungstenite::Message as WsMessage;
use tokio_tungstenite::tungstenite::handshake::server::{ErrorResponse, Request, Response};

use super::led_socket::LedSocket;
use super::pairing;
use super::spotify::{ISpotify, SpotifyError};

#[derive(thiserror::Error, Debug)]
pub(crate) enum SongPipelineError {
    #[error("Track has no cover")]
    NoCover,
    #[error("Failed to connect to LED")]
    LEDConnectionError,
    #[error("Failed to send to LED")]
    LEDSendError,
    #[error("Failed to fetch url")]
    HTTPClientError(#[from] reqwest::Error),
    #[error("Unexpected dealer message")]
    UnexpectedDealerMessage,
    #[error("Spotify error")]
    SpotifyError(#[from] SpotifyError),
    #[error("Failed to decode jpg")]
    JPGError(#[from] image::ImageError),
    #[error("Unknown command")]
    UnknownCommand,
    #[error("Command is ill-formed")]
    IllFormedCommand,
    #[error("Other")]
    Other,
}

enum LoopStatus {
    Continue,
    Break,
}

#[cfg_attr(test, mockall::automock)]
#[async_trait]
pub(crate) trait Controller: Send + Sync {
    async fn connect(&mut self, hostname: &str) -> Result<(), SongPipelineError>;
    async fn authorize(
        &mut self,
        auth_code: &str,
        code_verifier: &str,
    ) -> Result<(), SongPipelineError>;
    fn disconnect(&mut self) -> Result<(), SongPipelineError>;
    async fn display_name(&self) -> Result<Option<String>, SongPipelineError>;
}

pub struct WsServer<C: Controller> {
    controller: C,
    processor: CommandProcessor,
}

pub type Pipeline<S> = WsServer<PipelineController<S>>;

impl<S> WsServer<PipelineController<S>>
where
    S: LedSocket + 'static,
{
    pub fn new(spotify: Arc<dyn ISpotify>) -> Self {
        Self {
            controller: PipelineController::new(spotify),
            processor: CommandProcessor::new(),
        }
    }
}

enum Event<S> {
    Message(Option<Result<WsMessage, tokio_tungstenite::tungstenite::Error>>),
    NewConnection(Option<WebSocketStream<S>>),
}

impl<C> WsServer<C>
where
    C: Controller,
{
    pub async fn serve(&mut self, listener: TcpListener) -> Result<()> {
        let (tx, rx) = mpsc::channel(8);
        tokio::spawn(async move {
            loop {
                let (stream, _) = match listener.accept().await {
                    Ok(accepted) => accepted,
                    Err(err) => {
                        log::warn!("failed to accept connection: {err}");
                        break;
                    }
                };
                match tokio_tungstenite::accept_async(stream).await {
                    Ok(ws) => {
                        if tx.send(ws).await.is_err() {
                            break;
                        }
                    }
                    Err(err) => log::warn!("WebSocket handshake failed: {err}"),
                }
            }
        });
        self.serve_from(rx).await
    }

    pub async fn serve_tls(
        &mut self,
        listener: TcpListener,
        tls: TlsAcceptor,
        fingerprint: [u8; 32],
    ) -> Result<()> {
        let (tx, rx) = mpsc::channel(8);
        tokio::spawn(async move {
            loop {
                let (stream, _) = match listener.accept().await {
                    Ok(accepted) => accepted,
                    Err(err) => {
                        log::warn!("failed to accept connection: {err}");
                        break;
                    }
                };
                let tls = tls.clone();
                let tx = tx.clone();
                tokio::spawn(async move {
                    match tls.accept(stream).await {
                        Ok(tls_stream) => Self::route_tls_stream(tls_stream, fingerprint, tx).await,
                        Err(err) => log::warn!("TLS handshake failed: {err}"),
                    }
                });
            }
        });
        self.serve_from(rx).await
    }

    async fn route_tls_stream(
        stream: TlsStream<TcpStream>,
        fingerprint: [u8; 32],
        tx: mpsc::Sender<WebSocketStream<TlsStream<TcpStream>>>,
    ) {
        let mut path = String::new();
        let callback = |req: &Request, resp: Response| -> Result<Response, ErrorResponse> {
            path = req.uri().path().to_string();
            Ok(resp)
        };

        match tokio_tungstenite::accept_hdr_async(stream, callback).await {
            Ok(ws) if path == "/pair" => pairing::run_pairing(ws, fingerprint).await,
            Ok(ws) => {
                let _ = tx.send(ws).await;
            }
            Err(err) => log::warn!("WebSocket handshake failed: {err}"),
        }
    }

    async fn serve_from<S>(&mut self, mut new_connections: mpsc::Receiver<WebSocketStream<S>>) -> Result<()>
    where
        S: AsyncRead + AsyncWrite + Unpin,
    {
        let mut current: Option<WebSocketStream<S>> = None;

        loop {
            match Self::next_event(&mut current, &mut new_connections).await {
                Event::Message(Some(Ok(message))) => {
                    let ws = current.as_mut().expect("message implies an active connection");
                    if let LoopStatus::Break = self.handle_message(message, ws).await {
                        current = None;
                    }
                }
                Event::Message(_) => {
                    current = None;
                }
                Event::NewConnection(Some(ws)) => {
                    Self::accept_new_connection(ws, &mut current);
                }
                Event::NewConnection(None) => return Ok(()),
            }
        }
    }

    async fn next_event<S>(
        current: &mut Option<WebSocketStream<S>>,
        new_connections: &mut mpsc::Receiver<WebSocketStream<S>>,
    ) -> Event<S>
    where
        S: AsyncRead + AsyncWrite + Unpin,
    {
        let next_message = async {
            match current.as_mut() {
                Some(ws) => ws.next().await,
                None => std::future::pending().await,
            }
        };

        tokio::select! {
            message = next_message => Event::Message(message),
            connection = new_connections.recv() => Event::NewConnection(connection),
        }
    }

    fn accept_new_connection<S>(ws: WebSocketStream<S>, current: &mut Option<WebSocketStream<S>>) {
        if current.is_some() {
            log::info!("New client connected, replacing previous session");
        }
        *current = Some(ws);
    }

    async fn handle_message<S>(&mut self, msg: WsMessage, ws: &mut WebSocketStream<S>) -> LoopStatus
    where
        S: AsyncRead + AsyncWrite + Unpin,
    {
        match msg {
            WsMessage::Text(command) => {
                let (reply, status) =
                    match self.processor.process(&mut self.controller, &command).await {
                        Ok(reply) => (reply, LoopStatus::Continue),
                        Err(reply) => (reply, LoopStatus::Break),
                    };
                let _ = ws.send(WsMessage::Text(reply.into())).await;
                status
            }
            WsMessage::Close(_) => LoopStatus::Break,
            _ => LoopStatus::Continue,
        }
    }
}

#[cfg(test)]
mod song_pipeline_tests;
