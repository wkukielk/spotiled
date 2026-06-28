use anyhow::Result;
use std::fmt::Debug;
use std::sync::Arc;

use futures_util::SinkExt;
use futures_util::StreamExt;
use image::GenericImageView;
use librespot::metadata::audio::AudioItem;
use librespot::playback::player::PlayerEventChannel;
use tokio::net::{TcpListener, TcpStream};
use tokio::sync::Mutex;
use tokio::task::JoinHandle;
use tokio_tungstenite::WebSocketStream;
use tokio_tungstenite::tungstenite::Message;

use crate::dom_color::get_dominating_color;
use crate::led_socket::{LedSocket, LedSocketConnector};
use crate::player_event_listener::{PlayerEventListener, SongChangeHandler};

fn log_on_error<T, E: Debug>(result: &Result<T, E>) {
    if let Err(err) = result {
        log::error!("error encountered: {:?}", err);
    }
}

#[derive(thiserror::Error, Debug)]
enum SongPipelineError {
    #[error("Song has no cover")]
    NoCover,
    #[error("Failed to connect")]
    ConnectionError,
    #[error("Unknown command")]
    UnknownCommand,
}

enum LoopStatus {
    Continue,
    Break,
}

struct CoverColorExtractor<S: LedSocket> {
    http: reqwest::Client,
    socket: Arc<Mutex<S>>,
}

impl<S: LedSocket> CoverColorExtractor<S> {
    fn new(socket: Arc<Mutex<S>>) -> Self {
        Self {
            http: reqwest::Client::new(),
            socket,
        }
    }

    async fn fetch_cover(&self, url: &str) -> Result<bytes::Bytes, reqwest::Error> {
        let res = self.http.get(url).send().await?;
        res.error_for_status()?.bytes().await
    }

    async fn on_song_changed_impl(&self, audio_item: AudioItem) -> Result<()> {
        let cover_opt = audio_item.covers.first();
        let cover = cover_opt.ok_or(SongPipelineError::NoCover)?;

        let jpg = self.fetch_cover(&cover.url).await?;
        let color = tokio::task::spawn_blocking(move || {
            let pixels = CoverColorExtractor::<S>::decode_jpg_to_rgb565(&jpg)?;
            Ok::<_, anyhow::Error>(get_dominating_color(&pixels))
        })
        .await??;
        log::error!("{:?}", color);

        let mut socket = self.socket.lock().await;
        socket.send_rgb(&color).await?;
        Ok(())
    }

    fn rgb565(r: u8, g: u8, b: u8) -> u16 {
        ((r as u16 & 0xf8) << 8) | ((g as u16 & 0xfc) << 3) | (b as u16 >> 3)
    }

    fn decode_jpg_to_rgb565(jpg: &[u8]) -> Result<Vec<u16>, image::ImageError> {
        let image = image::load_from_memory_with_format(jpg, image::ImageFormat::Jpeg)?;
        Ok(image
            .pixels()
            .map(|(_, _, px)| CoverColorExtractor::<S>::rgb565(px[0], px[1], px[2]))
            .collect())
    }
}

impl<S: LedSocket> SongChangeHandler for CoverColorExtractor<S> {
    async fn on_song_changed(&self, audio_item: AudioItem) {
        log::info!("song changed: {}", audio_item.name);
        log_on_error(&self.on_song_changed_impl(audio_item).await);
    }
}

pub struct SongPipeline<C, F>
where
    C: LedSocketConnector,
    F: Fn() -> PlayerEventChannel + Send + Sync,
{
    subscribe: F,
    connector: C,
    active: Option<JoinHandle<()>>,
}

impl<C, F> SongPipeline<C, F>
where
    C: LedSocketConnector,
    F: Fn() -> PlayerEventChannel + Send + Sync,
{
    pub fn new(connector: C, subscribe: F) -> Self {
        Self {
            subscribe,
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
                Err(_err) => {
                    log::warn!("Error message");
                    break;
                }
            }
        }
    }

    async fn handle_message(
        &mut self,
        msg: Message,
        ws: &mut WebSocketStream<TcpStream>,
    ) -> LoopStatus {
        match msg {
            Message::Text(command) => match self.handle_command(&command).await {
                Ok(_) => LoopStatus::Continue,
                Err(err) => {
                    let _ = ws.send(Message::Text(err.to_string().into())).await;
                    LoopStatus::Break
                }
            },
            Message::Close(_) => LoopStatus::Break,
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
            .map_err(|_e| SongPipelineError::ConnectionError)?;
        let extractor = CoverColorExtractor::new(Arc::new(Mutex::new(socket)));
        let mut listener = PlayerEventListener::new((self.subscribe)());

        self.active = Some(tokio::spawn(async move {
            listener.run(&extractor).await;
        }));
        Ok(())
    }

    fn disconnect(&mut self) -> Result<(), SongPipelineError> {
        if let Some(task) = self.active.take() {
            task.abort();
        };
        Ok(())
    }
}

#[cfg(test)]
mod song_pipeline_tests;
