use super::*;

fn real_spotify_session() -> (tokio::runtime::Runtime, Spotify) {
    let rt = tokio::runtime::Runtime::new().expect("failed to build runtime");
    let mut spotify = Spotify::new();
    spotify
        .init(&rt)
        .expect("failed to connect to Spotify - requires valid cached credentials in .cache");
    (rt, spotify)
}

#[test]
#[ignore = "hits the real Spotify backend; run with `cargo test -- --ignored`"]
fn init_connects_and_starts_dealer() {
    let (_rt, spotify) = real_spotify_session();

    assert!(spotify.get_playback_event_stream().is_ok());
}

#[test]
#[ignore = "hits the real Spotify backend; run with `cargo test -- --ignored`"]
fn get_audio_item_fetches_real_track_metadata() {
    let (rt, spotify) = real_spotify_session();
    let uri = "spotify:track:4PTG3Z6ehGkBFwjybzWkR8"; // Never Gonna Give You Up

    let audio_item = rt
        .block_on(spotify.get_audio_item(uri))
        .expect("failed to fetch audio item from Spotify");

    assert_eq!(audio_item.uri, uri);
    assert!(!audio_item.name.is_empty());
    assert!(!audio_item.covers.is_empty());
}
