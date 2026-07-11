use std::net::SocketAddr;
use std::time::Duration;

use tokio::sync::mpsc;
use tokio_tungstenite::MaybeTlsStream;
use tokio_tungstenite::connect_async;

use super::*;

async fn spawn_server(controller: MockController) -> SocketAddr {
    let listener = TcpListener::bind("127.0.0.1:0").await.unwrap();
    let addr = listener.local_addr().unwrap();
    let mut pipeline = SongPipeline { controller };

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
    controller.expect_connect().times(1).return_once(move || {
        let _ = tx.send(());
        Ok(())
    });

    let addr = spawn_server(controller).await;
    let mut ws = connect_client(addr).await;

    ws.send(WsMessage::Text("connect".into())).await.unwrap();
    tokio::time::timeout(Duration::from_secs(1), rx.recv())
        .await
        .expect("timed out waiting for controller.connect() to be called")
        .expect("channel closed before signaling");

    // The connection should still be alive: the server keeps reading, so an
    // unknown command sent afterwards gets a normal error reply rather than
    // silence from an already-dropped connection.
    ws.send(WsMessage::Text("unknown".into())).await.unwrap();
    expect_text(&mut ws, "Unknown command").await;
}

#[tokio::test]
async fn connect_failure_replies_with_error_and_closes_connection() {
    let mut controller = MockController::new();
    controller
        .expect_connect()
        .times(1)
        .return_once(|| Err(SongPipelineError::LEDConnectionError));

    let addr = spawn_server(controller).await;
    let mut ws = connect_client(addr).await;

    ws.send(WsMessage::Text("connect".into())).await.unwrap();
    expect_text(&mut ws, "Failed to connect to LED").await;
    expect_connection_closed(&mut ws).await;
}

#[tokio::test]
async fn disconnect_command_invokes_controller_and_keeps_connection_open() {
    let (tx, mut rx) = mpsc::unbounded_channel();
    let mut controller = MockController::new();
    controller
        .expect_disconnect()
        .times(1)
        .return_once(move || {
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

    ws.send(WsMessage::Text("unknown".into())).await.unwrap();
    expect_text(&mut ws, "Unknown command").await;
}

#[tokio::test]
async fn unknown_command_replies_with_error_and_closes_connection() {
    // No expectations are set on the controller: an unknown command must
    // never reach it.
    let controller = MockController::new();

    let addr = spawn_server(controller).await;
    let mut ws = connect_client(addr).await;

    ws.send(WsMessage::Text("banana".into())).await.unwrap();
    expect_text(&mut ws, "Unknown command").await;
    expect_connection_closed(&mut ws).await;
}
