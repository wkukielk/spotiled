use std::net::SocketAddr;
use std::time::Duration;

use tokio::sync::mpsc;
use tokio_tungstenite::MaybeTlsStream;
use tokio_tungstenite::connect_async;

use super::*;

async fn spawn_server(controller: MockController) -> SocketAddr {
    let listener = TcpListener::bind("127.0.0.1:0").await.unwrap();
    let addr = listener.local_addr().unwrap();
    let mut pipeline = WsServer {
        controller,
        processor: CommandProcessor::new(),
    };

    tokio::spawn(async move {
        let _ = pipeline.serve(listener).await;
    });

    addr
}

async fn connect_client(addr: SocketAddr) -> WebSocketStream<MaybeTlsStream<TcpStream>> {
    let (ws, _) = connect_async(format!("ws://{addr}")).await.unwrap();
    ws
}

async fn expect_text(ws: &mut WebSocketStream<MaybeTlsStream<TcpStream>>, expected: &str) {
    let msg = tokio::time::timeout(Duration::from_secs(1), ws.next())
        .await
        .expect("timed out waiting for a reply")
        .expect("connection closed unexpectedly")
        .expect("websocket error");

    assert_eq!(msg, WsMessage::Text(expected.into()));
}

async fn expect_connection_closed(ws: &mut WebSocketStream<MaybeTlsStream<TcpStream>>) {
    let msg = tokio::time::timeout(Duration::from_secs(1), ws.next())
        .await
        .expect("timed out waiting for the connection to close");

    assert!(
        matches!(msg, None | Some(Err(_)) | Some(Ok(WsMessage::Close(_)))),
        "expected connection to close, got {msg:?}"
    );
}

#[tokio::test]
async fn connect_command_invokes_controller_and_keeps_connection_open() {
    let (tx, mut rx) = mpsc::unbounded_channel();
    let mut controller = MockController::new();
    controller.expect_connect().times(1).return_once(move |_| {
        let _ = tx.send(());
        Ok(())
    });
    // The connection is left open when the test ends, so cleanup may or may
    // not run before the runtime shuts down; either is fine here.
    controller.expect_disconnect().returning(|| Ok(()));

    let addr = spawn_server(controller).await;
    let mut ws = connect_client(addr).await;

    ws.send(WsMessage::Text("connect spotiled-esp32.local".into()))
        .await
        .unwrap();
    tokio::time::timeout(Duration::from_secs(1), rx.recv())
        .await
        .expect("timed out waiting for controller.connect() to be called")
        .expect("channel closed before signaling");
    expect_text(&mut ws, r#"{"ack":"ok"}"#).await;

    // The connection should still be alive: the server keeps reading, so an
    // unknown command sent afterwards gets a normal error reply rather than
    // silence from an already-dropped connection.
    ws.send(WsMessage::Text("unknown".into())).await.unwrap();
    expect_text(&mut ws, r#"{"error":"Unknown command"}"#).await;
}

#[tokio::test]
async fn connect_failure_replies_with_error_and_closes_connection() {
    let mut controller = MockController::new();
    controller
        .expect_connect()
        .times(1)
        .return_once(|_| Err(SongPipelineError::LEDConnectionError));
    // The connection closing runs cleanup, which calls disconnect() even
    // though connect() never succeeded — PipelineController's disconnect is
    // a no-op when nothing is active.
    controller.expect_disconnect().times(1).return_once(|| Ok(()));

    let addr = spawn_server(controller).await;
    let mut ws = connect_client(addr).await;

    ws.send(WsMessage::Text("connect spotiled-esp32.local".into()))
        .await
        .unwrap();
    expect_text(&mut ws, r#"{"error":"Failed to connect to LED"}"#).await;
    expect_connection_closed(&mut ws).await;
}

#[tokio::test]
async fn disconnect_command_invokes_controller_and_keeps_connection_open() {
    let (tx, mut rx) = mpsc::unbounded_channel();
    let mut controller = MockController::new();
    // Not pinned to a single call: besides the explicit "disconnect"
    // command, cleanup may also call disconnect() again if the connection
    // closes before the test ends.
    controller.expect_disconnect().returning(move || {
        let _ = tx.send(());
        Ok(())
    });

    let addr = spawn_server(controller).await;
    let mut ws = connect_client(addr).await;

    ws.send(WsMessage::Text("disconnect".into())).await.unwrap();
    tokio::time::timeout(Duration::from_secs(1), rx.recv())
        .await
        .expect("timed out waiting for controller.disconnect() to be called")
        .expect("channel closed before signaling");
    expect_text(&mut ws, r#"{"ack":"ok"}"#).await;

    ws.send(WsMessage::Text("unknown".into())).await.unwrap();
    expect_text(&mut ws, r#"{"error":"Unknown command"}"#).await;
}

#[tokio::test]
async fn unknown_command_replies_with_error_and_closes_connection() {
    // No connect expectation: an unknown command must never reach it.
    // disconnect() is still expected once, from cleanup after the
    // resulting close is handled.
    let mut controller = MockController::new();
    controller.expect_disconnect().times(1).return_once(|| Ok(()));

    let addr = spawn_server(controller).await;
    let mut ws = connect_client(addr).await;

    ws.send(WsMessage::Text("banana".into())).await.unwrap();
    expect_text(&mut ws, r#"{"error":"Unknown command"}"#).await;
    expect_connection_closed(&mut ws).await;
}

#[tokio::test]
async fn client_disconnecting_without_explicit_disconnect_command_cleans_up_controller() {
    let (tx, mut rx) = mpsc::unbounded_channel();
    let mut controller = MockController::new();
    controller.expect_connect().times(1).return_once(|_| Ok(()));
    controller
        .expect_disconnect()
        .times(1)
        .return_once(move || {
            let _ = tx.send(());
            Ok(())
        });

    let addr = spawn_server(controller).await;
    let mut ws = connect_client(addr).await;

    ws.send(WsMessage::Text("connect spotiled-esp32.local".into()))
        .await
        .unwrap();
    expect_text(&mut ws, r#"{"ack":"ok"}"#).await;

    // Simulate the client vanishing (crash, network drop, ...) instead of
    // sending "disconnect".
    drop(ws);

    tokio::time::timeout(Duration::from_secs(1), rx.recv())
        .await
        .expect("timed out waiting for controller.disconnect() to be called after client left")
        .expect("channel closed before signaling");
}
