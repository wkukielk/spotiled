use super::*;

fn real_spotify_session() -> (tokio::runtime::Runtime, Spotify) {
    let rt = tokio::runtime::Runtime::new().expect("failed to build runtime");
    let spotify = Spotify::new();
    (rt, spotify)
}

#[test]
#[ignore = "hits the real Spotify backend; run with `cargo test -- --ignored`"]
fn get_playback_event_stream_connects_lazily_using_cached_credentials() {
    let (rt, spotify) = real_spotify_session();

    let result = rt.block_on(spotify.get_playback_event_stream());

    assert!(
        result.is_ok(),
        "requires valid cached credentials in .cache"
    );
}

#[test]
#[ignore = "hits the real Spotify backend; run with `cargo test -- --ignored`"]
fn get_audio_item_fetches_real_track_metadata() {
    let (rt, spotify) = real_spotify_session();
    let uri = "spotify:track:4PTG3Z6ehGkBFwjybzWkR8"; // Never Gonna Give You Up

    let audio_item = rt
        .block_on(spotify.get_audio_item(uri))
        .expect("failed to fetch audio item from Spotify");

    assert!(!audio_item.covers.is_empty());
}
