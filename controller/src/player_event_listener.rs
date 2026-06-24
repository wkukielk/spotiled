use librespot::{
    metadata::audio::AudioItem,
    playback::player::{PlayerEvent, PlayerEventChannel},
};

pub trait SongChangeHandler {
    async fn on_song_changed(&self, audio_item: AudioItem);
}

pub struct PlayerEventListener<H: SongChangeHandler> {
    events: PlayerEventChannel,
    handler: H,
}

impl<H: SongChangeHandler> PlayerEventListener<H> {
    pub fn new(events: PlayerEventChannel, handler: H) -> Self {
        Self { events, handler }
    }

    pub async fn run(mut self) {
        while let Some(event) = self.events.recv().await {
            if let PlayerEvent::TrackChanged { audio_item } = event {
                self.handler.on_song_changed(*audio_item).await;
            }
        }
    }
}
