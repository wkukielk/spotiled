use super::spotify::SpotifyError;
use futures_util::StreamExt;
use librespot::core::dealer::manager::BoxedStreamResult;
use librespot::core::dealer::protocol::Message;

pub trait SongChangeHandler {
    fn on_song_changed(
        &mut self,
        msg: Result<Message, SpotifyError>,
    ) -> impl Future<Output = ()> + Send;
}

pub struct PlayerEventListener {
    stream: BoxedStreamResult<Message>,
}

impl PlayerEventListener {
    pub fn new(stream: BoxedStreamResult<Message>) -> Self {
        Self { stream }
    }

    pub async fn run(mut self, mut handler: impl SongChangeHandler) {
        while let Some(msg) = self.stream.next().await {
            let mapped_msg = msg.map_err(SpotifyError::Librespot);
            handler.on_song_changed(mapped_msg).await;
        }
    }
}
