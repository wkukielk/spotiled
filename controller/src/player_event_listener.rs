use librespot::{
    metadata::audio::AudioItem,
    playback::{
        audio_backend,
        convert::Converter,
        decoder::AudioPacket,
        player::{PlayerEvent, PlayerEventChannel},
    },
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

pub struct MySink;

impl MySink {
    pub fn new() -> Self {
        MySink
    }
}

impl audio_backend::Sink for MySink {
    fn write(&mut self, _: AudioPacket, _: &mut Converter) -> audio_backend::SinkResult<()> {
        Ok(())
    }
}
