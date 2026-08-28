use std::sync::{Arc, Mutex};

use futures_util::stream;
use mockall::predicate::eq;

use super::*;
use crate::led_socket::MockLedSocket;
use crate::spotify::{MockISpotify, SpotifyError};

// `MockLedSocket::connect_context()` expectations are global (mockall's
// static-method mocking has no per-test isolation), so tests that set them
// must be serialized against each other or they race across threads.
static CONNECT_MTX: Mutex<()> = Mutex::new(());

fn lock_connect_context() -> std::sync::MutexGuard<'static, ()> {
    CONNECT_MTX.lock().unwrap_or_else(|e| e.into_inner())
}

fn controller_with(spotify: MockISpotify) -> PipelineController<MockLedSocket> {
    PipelineController::new(Arc::new(spotify))
}

fn controller() -> PipelineController<MockLedSocket> {
    controller_with(MockISpotify::new())
}

#[tokio::test]
async fn connect_is_noop_when_already_active() {
    let mut controller = controller();
    controller.active = Some(ActiveSession::new(
        tokio::spawn(async {
            tokio::time::sleep(std::time::Duration::from_secs(60)).await;
        }),
        "example.local",
    ));

    let result = controller.connect("example.local").await;

    assert!(result.is_ok());
}

#[tokio::test]
async fn connect_reconnects_when_previous_task_already_finished() {
    let _guard = lock_connect_context();
    let connect_ctx = MockLedSocket::connect_context();
    connect_ctx
        .expect()
        .return_once(|_| Ok(MockLedSocket::default()));

    let mut spotify = MockISpotify::new();
    spotify
        .expect_get_playback_event_stream()
        .times(1)
        .return_once(|| Ok(Box::pin(stream::empty())));

    let mut controller = controller_with(spotify);
    let handle = tokio::spawn(async {});
    while !handle.is_finished() {
        tokio::task::yield_now().await;
    }
    controller.active = Some(ActiveSession::new(handle, "example.local"));

    let result = controller.connect("example.local").await;

    assert!(result.is_ok());
    assert!(controller.active.is_some());
}

#[tokio::test]
async fn connect_returns_led_connection_error_when_socket_fails() {
    let _guard = lock_connect_context();
    let connect_ctx = MockLedSocket::connect_context();
    connect_ctx
        .expect()
        .return_once(|_| Err(anyhow::anyhow!("boom")));

    let mut controller = controller();

    let result = controller.connect("example.local").await;

    assert!(matches!(
        result,
        Err(SongPipelineError::LEDConnectionError)
    ));
}

#[tokio::test]
async fn connect_propagates_error_when_playback_stream_unavailable() {
    let _guard = lock_connect_context();
    let connect_ctx = MockLedSocket::connect_context();
    connect_ctx
        .expect()
        .return_once(|_| Ok(MockLedSocket::default()));

    let mut spotify = MockISpotify::new();
    spotify
        .expect_get_playback_event_stream()
        .times(1)
        .return_once(|| Err(SpotifyError::NoCache));

    let mut controller = controller_with(spotify);

    let result = controller.connect("example.local").await;

    assert!(matches!(result, Err(SongPipelineError::SpotifyError(_))));
}

#[tokio::test]
async fn connect_spawns_listener_on_success() {
    let _guard = lock_connect_context();
    let connect_ctx = MockLedSocket::connect_context();
    connect_ctx
        .expect()
        .return_once(|_| Ok(MockLedSocket::default()));

    let mut spotify = MockISpotify::new();
    spotify
        .expect_get_playback_event_stream()
        .times(1)
        .return_once(|| Ok(Box::pin(stream::empty())));

    let mut controller = controller_with(spotify);

    let result = controller.connect("example.local").await;

    assert!(result.is_ok());
    assert!(controller.active.is_some());
}

#[tokio::test]
async fn disconnect_clears_active_task() {
    let mut controller = controller();
    controller.active = Some(ActiveSession::new(
        tokio::spawn(async {
            tokio::time::sleep(std::time::Duration::from_secs(60)).await;
        }),
        "example.local",
    ));

    let result = controller.disconnect();

    assert!(result.is_ok());
    assert!(controller.active.is_none());
}

#[test]
fn disconnect_is_noop_without_active_task() {
    let mut controller = controller();

    let result = controller.disconnect();

    assert!(result.is_ok());
}

#[test]
fn status_delegates_to_spotify() {
    let mut spotify = MockISpotify::new();
    spotify
        .expect_status()
        .times(1)
        .return_once(|| Some("someuser".to_string()));

    let controller = controller_with(spotify);

    assert_eq!(controller.status(), Some("someuser".to_string()));
}

#[tokio::test]
async fn authorize_passes_code_and_verifier_to_spotify() {
    let mut spotify = MockISpotify::new();
    spotify
        .expect_authorize()
        .with(eq("auth-code"), eq("code-verifier"))
        .times(1)
        .return_once(|_, _| Ok(()));

    let mut controller = controller_with(spotify);

    let result = controller.authorize("auth-code", "code-verifier").await;

    assert!(result.is_ok());
}

#[tokio::test]
async fn authorize_propagates_spotify_error() {
    let mut spotify = MockISpotify::new();
    spotify
        .expect_authorize()
        .times(1)
        .return_once(|_, _| Err(SpotifyError::NoCache));

    let mut controller = controller_with(spotify);

    let result = controller.authorize("auth-code", "code-verifier").await;

    assert!(matches!(result, Err(SongPipelineError::SpotifyError(_))));
}
