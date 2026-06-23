use std::fmt::Debug;

use image::GenericImageView;
use librespot::metadata::audio::AudioItem;

use crate::dom_color::get_dominating_color;
use crate::player_event_listener::SongChangeHandler;

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

fn log_and_extract<T, E: Debug>(result: Result<T, E>) -> Option<T> {
    match result {
        Ok(value) => Some(value),
        Err(err) => {
            log::error!("Algorithm error encountered: {:?}", err);
            None
        }
    }
}

pub struct SongPipeline {
    http: reqwest::Client,
}

impl SongPipeline {
    pub fn new() -> Self {
        Self {
            http: reqwest::Client::new(),
        }
    }

    async fn fetch_cover(&self, url: &str) -> Result<bytes::Bytes, reqwest::Error> {
        self.http.get(url).send().await?.bytes().await
    }
}

impl SongChangeHandler for SongPipeline {
    async fn on_song_changed(&self, audio_item: AudioItem) {
        log::info!("song changed: {}", audio_item.name);

        let Some(cover) = audio_item.covers.first() else {
            return;
        };

        let result = self.fetch_cover(&cover.url).await;
        let Some(jpg) = log_and_extract(result) else { return; };

        let result = decode_jpg_to_rgb565(&jpg);
        let Some(pixels) = log_and_extract(result) else { return; };

        let color = get_dominating_color(&pixels);
        println!("color: {:?}", color);
    }
}
