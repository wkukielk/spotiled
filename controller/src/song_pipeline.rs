use librespot::metadata::audio::AudioItem;

use crate::player_event_listener::SongChangeHandler;

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

        match self.fetch_cover(&cover.url).await {
            Ok(bytes) => log::info!("fetched cover art ({} bytes)", bytes.len()),
            Err(err) => log::warn!("failed to fetch cover art: {err}"),
        }
    }
}
