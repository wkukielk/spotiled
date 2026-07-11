use super::SongPipelineError;
use crate::dom_color::JPEGDecoder;
use crate::dom_color::get_jpg_dominating_color;
use crate::led_socket::LedSocket;
use crate::spotify::{ISpotify, SpotifyError, playback_event_listener::PlaybackEventHandler};
use librespot::core::dealer::protocol::{Message, PayloadValue};
use std::fmt::Debug;
use std::marker::PhantomData;
use std::sync::Arc;

fn log_on_error<T, E: Debug>(result: &Result<T, E>) {
    if let Err(err) = result {
        log::error!("error encountered: {:?}", err);
    }
}

#[cfg_attr(test, mockall::automock)]
pub trait CoverFetcher: Sync + Send {
    fn new(spotify: Arc<dyn ISpotify>) -> Self;
    fn fetch(
        &self,
        message: Result<Message, SpotifyError>,
    ) -> impl std::future::Future<Output = Result<bytes::Bytes, SongPipelineError>> + Send;
}

pub struct DftCoverFetcher {
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

pub struct SongHandler<F: CoverFetcher, S: LedSocket, D: JPEGDecoder> {
    cover_fetcher: F,
    socket: S,
    decoder: PhantomData<D>,
}

impl<F: CoverFetcher, S: LedSocket, D: JPEGDecoder> SongHandler<F, S, D> {
    pub fn new(spotify: Arc<dyn ISpotify>, socket: S) -> Self {
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

#[cfg(test)]
pub mod song_handler_tests;
