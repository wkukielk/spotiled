use anyhow::Result;
use std::fmt::Debug;
use std::sync::Arc;

use futures_util::{SinkExt, StreamExt};
use librespot::core::dealer::protocol::{Message, PayloadValue};
use librespot::metadata::audio::AudioItem;
use tokio::net::{TcpListener, TcpStream};
use tokio::task::JoinHandle;
use tokio_tungstenite::WebSocketStream;
use tokio_tungstenite::tungstenite::Message as WsMessage;

use crate::dom_color::{decode_jpg_to_rgb565, get_dominating_color};
use crate::led_socket::{LedSocket, LedSocketConnector};
use crate::player_event_listener::{PlayerEventListener, SongChangeHandler};
use crate::spotify::{Spotify, SpotifyError};

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
    #[error("Failed to connect")]
    ConnectionError,
    #[error("Spotify error")]
    SpotifyError(#[from] SpotifyError),
    #[error("Unknown command")]
    UnknownCommand,
}

enum LoopStatus {
    Continue,
    Break,
}

struct CoverColorExtractor<S: LedSocket> {
    http: reqwest::Client,
    spotify: Arc<Spotify>,
    socket: S,
}

impl<S: LedSocket> CoverColorExtractor<S> {
    fn new(spotify: Arc<Spotify>, socket: S) -> Self {
        Self {
            http: reqwest::Client::new(),
            spotify,
            socket,
        }
    }

    async fn fetch_cover(&self, url: &str) -> Result<bytes::Bytes, reqwest::Error> {
        self.http.get(url).send().await?.error_for_status()?.bytes().await
    }

    async fn on_track_changed(&mut self, audio_item: AudioItem) -> Result<()> {
        let cover = audio_item
            .covers
            .first()
            .ok_or(SongPipelineError::NoCover)?;
        let jpg = self.fetch_cover(&cover.url).await?;
        let color = tokio::task::spawn_blocking(move || {
            let pixels = decode_jpg_to_rgb565(&jpg)?;
            Ok::<_, anyhow::Error>(get_dominating_color(&pixels))
        })
        .await??;
        self.socket.send_rgb(&color).await?;
        Ok(())
    }
}

impl<S: LedSocket> SongChangeHandler for CoverColorExtractor<S> {
    async fn on_song_changed(&mut self, msg: Result<Message, SpotifyError>) {
        let Ok(msg) = msg else { return };
        let PayloadValue::Raw(ref bytes) = msg.payload else {
            return;
        };
        let Some(uri) = extract_track_uri(bytes) else {
            return;
        };
        match self.spotify.get_audio_item(&uri).await {
            Ok(item) => log_on_error(&self.on_track_changed(item).await),
            Err(e) => log::error!("Metadata error: {e}"),
        }
    }
}

pub struct SongPipeline<C>
where
    C: LedSocketConnector,
{
    spotify: Arc<Spotify>,
    connector: C,
    active: Option<JoinHandle<()>>,
}

impl<C> SongPipeline<C>
where
    C: LedSocketConnector,
{
    pub fn new(spotify: Spotify, connector: C) -> Self {
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
        if self.active.is_some() {
            return Ok(());
        }
        let socket = self
            .connector
            .connect()
            .await
            .map_err(|_| SongPipelineError::ConnectionError)?;
        let extractor = CoverColorExtractor::new(Arc::clone(&self.spotify), socket);
        let stream = self.spotify.get_stream()?;
        let event_listener = PlayerEventListener::new(stream);
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
