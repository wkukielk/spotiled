use anyhow::Result;
use std::fmt::Debug;
use std::marker::PhantomData;
use std::sync::Arc;

use futures_util::{SinkExt, StreamExt};
use librespot::core::dealer::protocol::{Message, PayloadValue};
use tokio::net::{TcpListener, TcpStream};
use tokio::task::JoinHandle;
use tokio_tungstenite::WebSocketStream;
use tokio_tungstenite::tungstenite::Message as WsMessage;

use crate::dom_color::{JPEGDecoder, StdJPEGDecoder};

use super::dom_color::get_jpg_dominating_color;
use super::led_socket::{LedSocket, LedSocketConnector};
use super::spotify::playback_event_listener::{PlaybackEventHandler, PlaybackEventListener};
use super::spotify::{ISpotify, SpotifyError};

fn log_on_error<T, E: Debug>(result: &Result<T, E>) {
    if let Err(err) = result {
        log::error!("error encountered: {:?}", err);
    }
}

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

#[cfg_attr(test, mockall::automock)]
trait CoverFetcher: Sync + Send {
    fn new(spotify: Arc<dyn ISpotify>) -> Self;
    fn fetch(
        &self,
        message: Result<Message, SpotifyError>,
    ) -> impl std::future::Future<Output = Result<bytes::Bytes, SongPipelineError>> + Send;
}

struct DftCoverFetcher {
    http: reqwest::Client,
    spotify: Arc<dyn ISpotify>,
}

impl DftCoverFetcher {
    fn extract_track_uri(bytes: &[u8]) -> Option<String> {
        let prefix = b"spotify:track:";
        let pos = bytes.windows(prefix.len()).position(|w| w == prefix)?;
        let rest = &bytes[pos..];
        let end = rest
            .iter()
            .position(|&b| !matches!(b, b'A'..=b'Z' | b'a'..=b'z' | b'0'..=b'9' | b':'))
            .unwrap_or(rest.len());
        String::from_utf8(rest[..end].to_vec()).ok()
    }

    fn extract_cover_uri(
        message: Result<Message, SpotifyError>,
    ) -> Result<String, SongPipelineError> {
        let msg = message?;
        let PayloadValue::Raw(ref bytes) = msg.payload else {
            return Err(SongPipelineError::UnexpectedDealerMessage);
        };
        Self::extract_track_uri(bytes).ok_or(SongPipelineError::UnexpectedDealerMessage)
    }

    async fn fetch_cover(&self, url: &str) -> Result<bytes::Bytes, reqwest::Error> {
        let response = self.http.get(url).send().await?;
        response.error_for_status()?.bytes().await
    }
}

impl CoverFetcher for DftCoverFetcher {
    fn new(spotify: Arc<dyn ISpotify>) -> Self {
        Self {
            http: reqwest::Client::new(),
            spotify,
        }
    }

    async fn fetch(
        &self,
        message: Result<Message, SpotifyError>,
    ) -> Result<bytes::Bytes, SongPipelineError> {
        let uri = Self::extract_cover_uri(message)?;
        let audio_item = self.spotify.get_audio_item(&uri).await?;
        let cover = audio_item.covers.last().ok_or(SongPipelineError::NoCover)?;
        self.fetch_cover(&cover.url)
            .await
            .map_err(|e| SongPipelineError::HTTPClientError(e))
    }
}

struct SongHandler<F: CoverFetcher, S: LedSocket, D: JPEGDecoder> {
    cover_fetcher: F,
    socket: S,
    decoder: PhantomData<D>,
}

impl<F: CoverFetcher, S: LedSocket, D: JPEGDecoder> SongHandler<F, S, D> {
    fn new(spotify: Arc<dyn ISpotify>, socket: S) -> Self {
        Self {
            cover_fetcher: F::new(spotify),
            socket,
            decoder: PhantomData,
        }
    }

    async fn on_song_changed_impl(
        &mut self,
        message: Result<Message, SpotifyError>,
    ) -> Result<(), SongPipelineError> {
        let cover_jpg = self.cover_fetcher.fetch(message).await?;
        let color = tokio::task::spawn_blocking(|| get_jpg_dominating_color::<D>(cover_jpg))
            .await
            .map_err(|_e| SongPipelineError::Other)??;
        self.socket
            .send_rgb(&color)
            .await
            .map_err(|_e| SongPipelineError::LEDSendError)?;
        Ok(())
    }
}

impl<F: CoverFetcher, S: LedSocket, D: JPEGDecoder> PlaybackEventHandler for SongHandler<F, S, D> {
    async fn on_song_changed(&mut self, msg: Result<Message, SpotifyError>) {
        log_on_error(&self.on_song_changed_impl(msg).await)
    }
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
