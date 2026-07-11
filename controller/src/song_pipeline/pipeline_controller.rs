use std::sync::Arc;

use tokio::task::JoinHandle;

use super::SongPipelineError;
use super::song_handler::{DftCoverFetcher, SongHandler};
use crate::dom_color::StdJPEGDecoder;
use crate::led_socket::{LedSocket, LedSocketConnector};
use crate::spotify::ISpotify;
use crate::spotify::playback_event_listener::PlaybackEventListener;

pub(super) struct PipelineController<S: LedSocket> {
    spotify: Arc<dyn ISpotify>,
    connector: LedSocketConnector<S>,
    active: Option<JoinHandle<()>>,
}

type Handler<S> = SongHandler<DftCoverFetcher, S, StdJPEGDecoder>;

impl<S> PipelineController<S>
where
    S: LedSocket + 'static,
{
    pub(super) fn new(spotify: Arc<dyn ISpotify>, connector: LedSocketConnector<S>) -> Self {
        Self {
            spotify,
            connector,
            active: None,
        }
    }

    pub(super) async fn create_handler(&self) -> Result<Handler<S>, SongPipelineError> {
        let socket_result = self.connector.connect().await;
        let socket = socket_result.map_err(|_| SongPipelineError::LEDConnectionError)?;
        let spoti = Arc::clone(&self.spotify);
        Ok(Handler::<S>::new(spoti, socket))
    }

    pub(super) async fn connect(&mut self) -> Result<(), SongPipelineError> {
        if self.active.is_some() {
            return Ok(());
        }

        let handler = self.create_handler().await?;
        let stream = self.spotify.get_playback_event_stream()?;
        let event_listener = PlaybackEventListener::new(stream);

        self.active = Some(tokio::spawn(async move {
            event_listener.run(handler).await;
        }));

        Ok(())
    }

    pub(super) fn disconnect(&mut self) -> Result<(), SongPipelineError> {
        if let Some(task) = self.active.take() {
            task.abort();
        }
        Ok(())
    }
}

#[cfg(test)]
mod pipeline_controller_tests;
