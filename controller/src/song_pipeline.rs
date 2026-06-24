use anyhow::Result;
use std::fmt::Debug;
use std::sync::Arc;

use image::GenericImageView;
use librespot::metadata::audio::AudioItem;

use librespot::playback::player::PlayerEventChannel;
use tokio::sync::Mutex;

use crate::dom_color::get_dominating_color;
use crate::led_socket::LedSocket;
use crate::player_event_listener::{PlayerEventListener, SongChangeHandler};

fn rgb565(r: u8, g: u8, b: u8) -> u16 {
    ((r as u16 & 0xf8) << 8) | ((g as u16 & 0xfc) << 3) | (b as u16 >> 3)
}

fn decode_jpg_to_rgb565(jpg: &[u8]) -> Result<Vec<u16>, image::ImageError> {
    let image = image::load_from_memory_with_format(jpg, image::ImageFormat::Jpeg)?;
    Ok(image
        .pixels()
        .map(|(_, _, px)| rgb565(px[0], px[1], px[2]))
        .collect())
}

fn log_on_error<T, E: Debug>(result: Result<T, E>) {
    if let Err(err) = result {
        log::error!("Algorithm error encountered: {:?}", err);
    }
}

#[derive(thiserror::Error, Debug)]
pub enum SongPipelineError {
    #[error("Song has no cover")]
    NoCover,
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
        let pixels = decode_jpg_to_rgb565(&jpg)?;
        let color = get_dominating_color(&pixels);

        let mut socket = self.socket.lock().await;
        socket.send_rgb(&color).await?;
        Ok(())
    }
}

impl<S: LedSocket> SongChangeHandler for CoverColorExtractor<S> {
    async fn on_song_changed(&self, audio_item: AudioItem) {
        log::info!("song changed: {}", audio_item.name);
        log_on_error(self.on_song_changed_impl(audio_item).await);
    }
}

pub struct SongPipeline<S: LedSocket> {
    listener: PlayerEventListener,
    extractor: CoverColorExtractor<S>,
    socket: Arc<Mutex<S>>,
}

impl<S: LedSocket> SongPipeline<S> {
    pub fn new(events: PlayerEventChannel, socket: S) -> Self {
        let socket = Arc::new(Mutex::new(socket));
        Self {
            listener: PlayerEventListener::new(events),
            extractor: CoverColorExtractor::new(socket.clone()),
            socket,
        }
    }

    pub async fn run(&mut self) {
        self.listener.run(&self.extractor).await
    }
}

#[cfg(test)]
mod song_pipeline_tests;
