mod song_handler;

use self::song_handler::{DftCoverFetcher, SongHandler};
use anyhow::Result;
use std::fmt::Debug;
use std::sync::Arc;

use futures_util::{SinkExt, StreamExt};
use tokio::net::{TcpListener, TcpStream};
use tokio::task::JoinHandle;
use tokio_tungstenite::WebSocketStream;
use tokio_tungstenite::tungstenite::Message as WsMessage;

use crate::dom_color::StdJPEGDecoder;

use super::led_socket::{LedSocket, LedSocketConnector};
use super::spotify::playback_event_listener::PlaybackEventListener;
use super::spotify::{ISpotify, SpotifyError};

#[derive(thiserror::Error, Debug)]
enum SongPipelineError {
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
    #[error("Other")]
    Other,
}

enum LoopStatus {
    Continue,
    Break,
}

pub struct SongPipeline<S>
where
    S: LedSocket,
{
    spotify: Arc<dyn ISpotify>,
    connector: LedSocketConnector<S>,
    active: Option<JoinHandle<()>>,
}

impl<S> SongPipeline<S>
where
    S: LedSocket + 'static,
{
    pub fn new(spotify: Arc<dyn ISpotify>, connector: LedSocketConnector<S>) -> Self {
        Self {
            spotify: spotify,
            connector,
            active: None,
        }
    }

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
            WsMessage::Text(command) => match self.handle_command(&command).await {
                Ok(_) => LoopStatus::Continue,
                Err(err) => {
                    let _ = ws.send(WsMessage::Text(err.to_string().into())).await;
                    LoopStatus::Break
                }
            },
            WsMessage::Close(_) => LoopStatus::Break,
            _ => LoopStatus::Continue,
        }
    }

    async fn handle_command(&mut self, command: &str) -> Result<(), SongPipelineError> {
        match command {
            "connect" => self.connect().await,
            "disconnect" => self.disconnect(),
            _other => Err(SongPipelineError::UnknownCommand),
        }
    }

    async fn connect(&mut self) -> Result<(), SongPipelineError> {
        log::info!("connect");
        if self.active.is_some() {
            return Ok(());
        }
        let socket_result = self.connector.connect().await;
        let socket = socket_result.map_err(|_| SongPipelineError::LEDConnectionError)?;

        let spoti = Arc::clone(&self.spotify);
        let handler = SongHandler::<DftCoverFetcher, S, StdJPEGDecoder>::new(spoti, socket);

        let stream = self.spotify.get_playback_event_stream()?;
        let event_listener = PlaybackEventListener::new(stream);
        self.active = Some(tokio::spawn(async move {
            event_listener.run(handler).await;
        }));
        Ok(())
    }

    fn disconnect(&mut self) -> Result<(), SongPipelineError> {
        if let Some(task) = self.active.take() {
            task.abort();
        }
        Ok(())
    }
}

#[cfg(test)]
mod song_pipeline_tests;
