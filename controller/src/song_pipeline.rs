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

impl<C> WsServer<C>
where
    C: Controller,
{
    pub async fn serve(&mut self, listener: TcpListener) -> Result<()> {
        loop {
            let (stream, _) = listener.accept().await?;
            let ws = tokio_tungstenite::accept_async(stream).await?;
            self.handle_client(ws).await;
        }
    }

    async fn handle_client(&mut self, mut ws: WebSocketStream<TcpStream>) {
        while let Some(message) = ws.next().await {
            match message {
                Ok(message) => {
                    if let LoopStatus::Break = self.handle_message(message, &mut ws).await {
                        break;
                    }
                }
                Err(_) => {
                    log::warn!("Error message");
                    break;
                }
            }
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
