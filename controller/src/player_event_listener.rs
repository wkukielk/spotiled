use librespot::{
    metadata::audio::AudioItem,
    playback::player::{PlayerEvent, PlayerEventChannel},
};

pub trait SongChangeHandler {
    async fn on_song_changed(&self, audio_item: AudioItem);
}

pub struct PlayerEventListener {
    events: PlayerEventChannel,
}

impl PlayerEventListener {
    pub fn new(events: PlayerEventChannel) -> Self {
        Self { events }
    }

    pub async fn run(&mut self, handler: &impl SongChangeHandler) {
        while let Some(event) = self.events.recv().await {
            if let PlayerEvent::TrackChanged { audio_item } = event {
                handler.on_song_changed(*audio_item).await;
            }
        }
    }
}
