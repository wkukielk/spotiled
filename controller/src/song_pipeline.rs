use anyhow::Result;
use std::fmt::Debug;
use std::marker::PhantomData;
use std::sync::Arc;

use futures_util::{SinkExt, StreamExt};
use librespot::core::dealer::protocol::{Message, PayloadValue};
use librespot::metadata::audio::AudioItem;
use tokio::net::{TcpListener, TcpStream};
use tokio::task::JoinHandle;
use tokio_tungstenite::WebSocketStream;
use tokio_tungstenite::tungstenite::Message as WsMessage;

use crate::dom_color::{JPEGDecoder, StdJPEGDecoder};

use super::dom_color::get_jpg_dominating_color;
use super::led_socket::{LedSocket, LedSocketConnector};
use super::spotify::playback_event_listener::{PlaybackEventHandler, PlaybackEventListener};
use super::spotify::{Spotify, SpotifyError};

fn log_on_error<T, E: Debug>(result: &Result<T, E>) {
    if let Err(err) = result {
        log::error!("error encountered: {:?}", err);
    }
}

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

struct CoverColorExtractor<S: LedSocket, D: JPEGDecoder> {
    http: reqwest::Client,
    spotify: Arc<Spotify>,
    socket: S,
    decoder: PhantomData<D>,
}

impl<S: LedSocket, D: JPEGDecoder> CoverColorExtractor<S, D> {
    fn new(spotify: Arc<Spotify>, socket: S) -> Self {
        Self {
            http: reqwest::Client::new(),
            spotify,
            socket,
            decoder: PhantomData,
        }
    }

    async fn fetch_cover(&self, url: &str) -> Result<bytes::Bytes, reqwest::Error> {
        let response = self.http.get(url).send().await?;
        response.error_for_status()?.bytes().await
    }

    async fn on_track_changed(&mut self, audio_item: AudioItem) -> Result<(), SongPipelineError> {
        let cover = audio_item.covers.last().ok_or(SongPipelineError::NoCover)?;
        let jpg = self.fetch_cover(&cover.url).await?;
        let color = tokio::task::spawn_blocking(|| get_jpg_dominating_color::<D>(jpg))
            .await
            .map_err(|_e| SongPipelineError::Other)??;
        self.socket
            .send_rgb(&color)
            .await
            .map_err(|_e| SongPipelineError::LEDSendError)?;
        Ok(())
    }

    async fn on_song_changed_impl(
        &mut self,
        message: Result<Message, SpotifyError>,
    ) -> Result<(), SongPipelineError> {
        let msg = message?;
        let PayloadValue::Raw(ref bytes) = msg.payload else {
            return Err(SongPipelineError::UnexpectedDealerMessage);
        };
        let Some(uri) = extract_track_uri(bytes) else {
            return Err(SongPipelineError::UnexpectedDealerMessage);
        };
        let audio_item = self.spotify.get_audio_item(&uri).await?;
        self.on_track_changed(audio_item).await
    }
}

impl<S: LedSocket, D: JPEGDecoder> PlaybackEventHandler for CoverColorExtractor<S, D> {
    async fn on_song_changed(&mut self, msg: Result<Message, SpotifyError>) {
        log_on_error(&self.on_song_changed_impl(msg).await)
    }
}

pub struct SongPipeline<S>
where
    S: LedSocket,
{
    spotify: Arc<Spotify>,
    connector: LedSocketConnector<S>,
    active: Option<JoinHandle<()>>,
}

impl<S> SongPipeline<S>
where
    S: LedSocket + 'static,
{
    pub fn new(spotify: Spotify, connector: LedSocketConnector<S>) -> Self {
        Self {
            spotify: Arc::new(spotify),
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
        log::error!("connect");
        if self.active.is_some() {
            return Ok(());
        }
        let socket_result = self.connector.connect().await;
        let socket = socket_result.map_err(|_| SongPipelineError::LEDConnectionError)?;

        let extractor =
            CoverColorExtractor::<S, StdJPEGDecoder>::new(Arc::clone(&self.spotify), socket);
        let stream = self.spotify.get_playback_event_stream()?;
        let event_listener = PlaybackEventListener::new(stream);
        self.active = Some(tokio::spawn(async move {
            event_listener.run(extractor).await;
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
