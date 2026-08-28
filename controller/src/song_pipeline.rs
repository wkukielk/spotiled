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
use tokio::net::{TcpListener, TcpStream};
use tokio_tungstenite::WebSocketStream;
use tokio_tungstenite::tungstenite::Message as WsMessage;

use super::led_socket::LedSocket;
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
    /// The connected Spotify account's username, or `None` if not connected.
    fn status(&self) -> Option<String>;
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

enum Event {
    Message(Option<Result<WsMessage, tokio_tungstenite::tungstenite::Error>>),
    NewConnection(std::io::Result<(TcpStream, std::net::SocketAddr)>),
}

impl<C> WsServer<C>
where
    C: Controller,
{
    pub async fn serve(&mut self, listener: TcpListener) -> Result<()> {
        let mut current: Option<WebSocketStream<TcpStream>> = None;

        loop {
            match Self::next_event(&mut current, &listener).await {
                Event::Message(Some(Ok(message))) => {
                    let ws = current.as_mut().expect("message implies an active connection");
                    if let LoopStatus::Break = self.handle_message(message, ws).await {
                        current = None;
                    }
                }
                Event::Message(_) => {
                    current = None;
                }
                Event::NewConnection(Ok((stream, _))) => {
                    Self::accept_new_connection(stream, &mut current).await;
                }
                Event::NewConnection(Err(err)) => return Err(err.into()),
            }
        }
    }

    async fn next_event(current: &mut Option<WebSocketStream<TcpStream>>, listener: &TcpListener) -> Event {
        let next_message = async {
            match current.as_mut() {
                Some(ws) => ws.next().await,
                None => std::future::pending().await,
            }
        };

        tokio::select! {
            message = next_message => Event::Message(message),
            accepted = listener.accept() => Event::NewConnection(accepted),
        }
    }

    async fn accept_new_connection(stream: TcpStream, current: &mut Option<WebSocketStream<TcpStream>>) {
        match tokio_tungstenite::accept_async(stream).await {
            Ok(new_ws) => {
                if current.is_some() {
                    log::info!("New client connected, replacing previous session");
                }
                *current = Some(new_ws);
            }
            Err(err) => log::warn!("WebSocket handshake failed: {err}"),
        }
    }

    async fn handle_message(
        &mut self,
        msg: WsMessage,
        ws: &mut WebSocketStream<TcpStream>,
    ) -> LoopStatus {
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
