use std::marker::PhantomData;
use std::sync::Arc;

use async_trait::async_trait;
use tokio::task::JoinHandle;

use super::Controller;
use super::SongPipelineError;
use super::song_handler::{DftCoverFetcher, SongHandler};
use crate::dom_color::StdJPEGDecoder;
use crate::led_socket::LedSocket;
use crate::spotify::ISpotify;
use crate::spotify::playback_event_listener::PlaybackEventListener;

const LED_SOCKET_PORT: u16 = 8000;

struct ActiveSession{
    pub handle: JoinHandle<()>,
    pub hostname: String,
}

impl ActiveSession {
    pub fn new(handle: JoinHandle<()>, hostname: &str) -> Self {
        Self {
            handle: handle,
            hostname: hostname.to_string()
        }
    }
}

impl Drop for ActiveSession {
    fn drop(&mut self) {
        self.handle.abort();
    }
}

pub(crate) struct PipelineController<S: LedSocket> {
    spotify: Arc<dyn ISpotify>,
    active: Option<ActiveSession>,
    _socket: PhantomData<S>,
}

type Handler<S> = SongHandler<DftCoverFetcher, S, StdJPEGDecoder>;

impl<S> PipelineController<S>
where
    S: LedSocket + 'static,
{
    pub(crate) fn new(spotify: Arc<dyn ISpotify>) -> Self {
        Self {
            spotify,
            active: None,
            _socket: PhantomData,
        }
    }

    async fn create_handler(&self, hostname: &str) -> Result<Handler<S>, SongPipelineError> {
        let url = format!("ws://{hostname}:{LED_SOCKET_PORT}/ws");
        let socket = S::connect(&url)
            .await
            .map_err(|_| SongPipelineError::LEDConnectionError)?;
        let spoti = Arc::clone(&self.spotify);
        Ok(Handler::<S>::new(spoti, socket))
    }
}

#[async_trait]
impl<S> Controller for PipelineController<S>
where
    S: LedSocket + 'static,
{
    async fn connect(&mut self, hostname: &str) -> Result<(), SongPipelineError> {
        if let Some(active) = &self.active && active.hostname == hostname {
            return Ok(());
        }
        log::info!("connect {hostname}");

        let handler = self.create_handler(hostname).await?;
        let stream = self.spotify.get_playback_event_stream().await?;
        let event_listener = PlaybackEventListener::new(stream);

        let handle = tokio::spawn(async move {
            event_listener.run(handler).await;
        });
        self.active = Some(ActiveSession::new(handle, hostname));

        Ok(())
    }

    async fn authorize(
        &mut self,
        auth_code: &str,
        code_verifier: &str,
    ) -> Result<(), SongPipelineError> {
        self.spotify.authorize(auth_code, code_verifier).await?;
        Ok(())
    }

    fn disconnect(&mut self) -> Result<(), SongPipelineError> {
        self.active = None;
        Ok(())
    }
}

#[cfg(test)]
mod pipeline_controller_tests;
