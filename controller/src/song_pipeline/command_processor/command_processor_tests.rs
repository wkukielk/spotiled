use mockall::predicate::eq;

use super::*;
use crate::song_pipeline::MockController;
use crate::spotify::SpotifyError;

#[tokio::test]
async fn connect_passes_hostname_and_returns_ack_json() {
    let mut controller = MockController::new();
    controller
        .expect_connect()
        .with(eq("spotiled-esp32.local"))
        .times(1)
        .return_once(|_| Ok(()));

    let mut processor = CommandProcessor::new();
    let result = processor
        .process(&mut controller, "connect spotiled-esp32.local")
        .await;

    assert_eq!(result, Ok(r#"{"ack":"ok"}"#.to_string()));
}

#[tokio::test]
async fn connect_failure_returns_error_json() {
    let mut controller = MockController::new();
    controller
        .expect_connect()
        .times(1)
        .return_once(|_| Err(SongPipelineError::LEDConnectionError));

    let mut processor = CommandProcessor::new();
    let result = processor
        .process(&mut controller, "connect spotiled-esp32.local")
        .await;

    assert_eq!(
        result,
        Err(r#"{"error":"Failed to connect to LED"}"#.to_string())
    );
}

#[tokio::test]
async fn connect_without_hostname_returns_error_json_without_touching_controller() {
    let mut controller = MockController::new();

    let mut processor = CommandProcessor::new();
    let result = processor.process(&mut controller, "connect").await;

    assert_eq!(
        result,
        Err(r#"{"error":"Command is ill-formed"}"#.to_string())
    );
}

#[tokio::test]
async fn disconnect_success_returns_ack_json() {
    let mut controller = MockController::new();
    controller
        .expect_disconnect()
        .times(1)
        .return_once(|| Ok(()));

    let mut processor = CommandProcessor::new();
    let result = processor.process(&mut controller, "disconnect").await;

    assert_eq!(result, Ok(r#"{"ack":"ok"}"#.to_string()));
}

#[tokio::test]
async fn authorize_passes_code_and_verifier_and_returns_ack_json() {
    let mut controller = MockController::new();
    controller
        .expect_authorize()
        .with(eq("auth-code"), eq("code-verifier"))
        .times(1)
        .return_once(|_, _| Ok(()));

    let mut processor = CommandProcessor::new();
    let result = processor
        .process(&mut controller, "authorize auth-code code-verifier")
        .await;

    assert_eq!(result, Ok(r#"{"ack":"ok"}"#.to_string()));
}

#[tokio::test]
async fn authorize_failure_returns_error_json() {
    let mut controller = MockController::new();
    controller
        .expect_authorize()
        .times(1)
        .return_once(|_, _| Err(SongPipelineError::SpotifyError(SpotifyError::NoCache)));

    let mut processor = CommandProcessor::new();
    let result = processor
        .process(&mut controller, "authorize auth-code code-verifier")
        .await;

    assert_eq!(result, Err(r#"{"error":"Spotify error"}"#.to_string()));
}

#[tokio::test]
async fn authorize_without_verifier_returns_error_json_without_touching_controller() {
    let mut controller = MockController::new();

    let mut processor = CommandProcessor::new();
    let result = processor
        .process(&mut controller, "authorize auth-code")
        .await;

    assert_eq!(
        result,
        Err(r#"{"error":"Command is ill-formed"}"#.to_string())
    );
}

#[tokio::test]
async fn unknown_command_returns_error_json_without_touching_controller() {
    let mut controller = MockController::new();

    let mut processor = CommandProcessor::new();
    let result = processor.process(&mut controller, "banana").await;

    assert_eq!(result, Err(r#"{"error":"Unknown command"}"#.to_string()));
}

#[test]
fn get_next_part_test() {
    let mut parts = "hello world".split_whitespace();
    let hello = get_next_part(&mut parts).unwrap();
    assert_eq!(hello, "hello");
}

#[test]
fn get_next_part_stop_iteration_test() {
    let mut parts = "hello world".split_whitespace();
    get_next_part(&mut parts).unwrap();
    get_next_part(&mut parts).unwrap();
    let result = get_next_part(&mut parts);
    assert!(matches!(result, Err(SongPipelineError::IllFormedCommand)));
}
