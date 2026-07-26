use mockall::predicate::eq;

use super::*;
use crate::song_pipeline::MockController;

#[tokio::test]
async fn connect_passes_hostname_and_returns_ack_json() {
    let mut controller = MockController::new();
    controller
        .expect_connect()
        .with(eq("spotiled-esp32.local"))
        .times(1)
        .return_once(|_| Ok(()));

    let mut processor = CommandProcessor::new(controller);
    let result = processor.process("connect spotiled-esp32.local").await;

    assert_eq!(result, Ok(r#"{"ack":"ok"}"#.to_string()));
}

#[tokio::test]
async fn connect_failure_returns_error_json() {
    let mut controller = MockController::new();
    controller
        .expect_connect()
        .times(1)
        .return_once(|_| Err(SongPipelineError::LEDConnectionError));

    let mut processor = CommandProcessor::new(controller);
    let result = processor.process("connect spotiled-esp32.local").await;

    assert_eq!(
        result,
        Err(r#"{"error":"Failed to connect to LED"}"#.to_string())
    );
}

#[tokio::test]
async fn connect_without_hostname_returns_error_json_without_touching_controller() {
    let controller = MockController::new();

    let mut processor = CommandProcessor::new(controller);
    let result = processor.process("connect").await;

    assert_eq!(
        result,
        Err(r#"{"error":"Missing hostname argument"}"#.to_string())
    );
}

#[tokio::test]
async fn disconnect_success_returns_ack_json() {
    let mut controller = MockController::new();
    controller
        .expect_disconnect()
        .times(1)
        .return_once(|| Ok(()));

    let mut processor = CommandProcessor::new(controller);
    let result = processor.process("disconnect").await;

    assert_eq!(result, Ok(r#"{"ack":"ok"}"#.to_string()));
}

#[tokio::test]
async fn unknown_command_returns_error_json_without_touching_controller() {
    let controller = MockController::new();

    let mut processor = CommandProcessor::new(controller);
    let result = processor.process("banana").await;

    assert_eq!(
        result,
        Err(r#"{"error":"Unknown command"}"#.to_string())
    );
}
