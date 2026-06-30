use super::SpotifyError;
use futures_util::StreamExt;
use librespot::core::dealer::manager::BoxedStreamResult;
use librespot::core::dealer::protocol::Message;

pub trait PlaybackEventHandler {
    fn on_song_changed(
        &mut self,
        msg: Result<Message, SpotifyError>,
    ) -> impl Future<Output = ()> + Send;
}

pub struct PlaybackEventListener {
    stream: BoxedStreamResult<Message>,
}

impl PlaybackEventListener {
    pub fn new(stream: BoxedStreamResult<Message>) -> Self {
        Self { stream }
    }

    pub async fn run(mut self, mut handler: impl PlaybackEventHandler) {
        while let Some(msg) = self.stream.next().await {
            let mapped_msg = msg.map_err(SpotifyError::Librespot);
            handler.on_song_changed(mapped_msg).await;
        }
    }
}
