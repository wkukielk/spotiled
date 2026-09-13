use super::*;
use crate::status::{self, ControllerStatus};
use serial_test::serial;
use tokio_tungstenite::{accept_async, client_async};

fn cert_fp() -> [u8; 32] {
    [7u8; 32]
}

#[test]
fn compute_sas_is_deterministic() {
    let (a, b, s, fp) = ([1u8; 32], [2u8; 32], [3u8; 32], [4u8; 32]);
    assert_eq!(compute_sas(&a, &b, &s, &fp), compute_sas(&a, &b, &s, &fp));
}

#[test]
fn compute_sas_depends_on_every_input() {
    let (a, b, s, fp) = ([1u8; 32], [2u8; 32], [3u8; 32], [4u8; 32]);
    let base = compute_sas(&a, &b, &s, &fp);
    assert_ne!(base, compute_sas(&[9u8; 32], &b, &s, &fp));
    assert_ne!(base, compute_sas(&a, &[9u8; 32], &s, &fp));
    assert_ne!(base, compute_sas(&a, &b, &[9u8; 32], &fp));
    assert_ne!(base, compute_sas(&a, &b, &s, &[9u8; 32]));
}

#[test]
fn compute_sas_is_six_grouped_digits() {
    let sas = compute_sas(&[0u8; 32], &[0u8; 32], &[0u8; 32], &[0u8; 32]);
    assert_eq!(sas.len(), 7);
    for (i, c) in sas.chars().enumerate() {
        if i == 3 {
            assert_eq!(c, ' ');
        } else {
            assert!(c.is_ascii_digit());
        }
    }
}

/// Drives the server's half of `run_pairing` over an in-memory duplex stream, returning a raw
/// `WebSocketStream` the test controls as the "client" — so each test can play out its own wire
/// sequence without a real TCP connection.
async fn spawn_pair() -> (
    WebSocketStream<tokio::io::DuplexStream>,
    tokio::task::JoinHandle<()>,
) {
    let (client_io, server_io) = tokio::io::duplex(8192);
    let handle = tokio::spawn(async move {
        let ws = accept_async(server_io).await.unwrap();
        run_pairing(ws, cert_fp()).await;
    });
    let (client_ws, _) = client_async("ws://localhost/pair", client_io).await.unwrap();
    (client_ws, handle)
}

/// Runs the client side of the handshake (commit, receive the server's key, reveal), returning
/// the negotiated shared secret and both public keys so a test can compute the expected SAS.
async fn handshake(
    ws: &mut WebSocketStream<tokio::io::DuplexStream>,
) -> ([u8; 32], [u8; 32], [u8; 32]) {
    let secret = EphemeralSecret::random();
    let client_pub = PublicKey::from(&secret);
    let commit: [u8; 32] = Sha256::digest(client_pub.as_bytes()).into();
    send_json(
        ws,
        &serde_json::json!({ "type": "commit", "hash": BASE64.encode(commit) }),
    )
    .await
    .unwrap();

    let server_pub_bytes = recv_reveal(ws).await.unwrap();

    send_json(
        ws,
        &serde_json::json!({ "type": "reveal", "pub": BASE64.encode(client_pub.as_bytes()) }),
    )
    .await
    .unwrap();

    let server_pub = PublicKey::from(server_pub_bytes);
    let shared = secret.diffie_hellman(&server_pub);
    (*client_pub.as_bytes(), server_pub_bytes, *shared.as_bytes())
}

#[tokio::test]
#[serial]
async fn confirming_completes_pairing() {
    let (mut ws, handle) = spawn_pair().await;
    let (client_pub, server_pub, shared) = handshake(&mut ws).await;

    let expected_sas = compute_sas(&client_pub, &server_pub, &shared, &cert_fp());
    let expected_status = ControllerStatus::Pairing { sas: expected_sas };
    timeout(Duration::from_secs(1), async {
        while status::get() != expected_status {
            tokio::time::sleep(Duration::from_millis(1)).await;
        }
    })
    .await
    .expect("server never published the pairing SAS");

    send_json(&mut ws, &serde_json::json!({ "type": "confirm" }))
        .await
        .unwrap();

    let reply = next_text(&mut ws).await.unwrap();
    assert_eq!(reply, r#"{"type":"paired"}"#);

    handle.await.unwrap();
    assert_eq!(status::get(), ControllerStatus::Running);
}

#[tokio::test]
#[serial]
async fn rejecting_replies_rejected() {
    let (mut ws, handle) = spawn_pair().await;
    handshake(&mut ws).await;

    send_json(&mut ws, &serde_json::json!({ "type": "reject" }))
        .await
        .unwrap();

    let reply = next_text(&mut ws).await.unwrap();
    assert_eq!(reply, r#"{"type":"rejected"}"#);

    handle.await.unwrap();
    assert_eq!(status::get(), ControllerStatus::Running);
}

#[tokio::test]
async fn revealing_a_key_that_does_not_match_the_commitment_aborts_the_connection() {
    let (mut ws, handle) = spawn_pair().await;

    let secret = EphemeralSecret::random();
    let committed_pub = PublicKey::from(&secret);
    let commit: [u8; 32] = Sha256::digest(committed_pub.as_bytes()).into();
    send_json(
        &mut ws,
        &serde_json::json!({ "type": "commit", "hash": BASE64.encode(commit) }),
    )
    .await
    .unwrap();

    recv_reveal(&mut ws).await.unwrap();

    // Reveal a different key than the one committed to.
    let other_secret = EphemeralSecret::random();
    let other_pub = PublicKey::from(&other_secret);
    send_json(
        &mut ws,
        &serde_json::json!({ "type": "reveal", "pub": BASE64.encode(other_pub.as_bytes()) }),
    )
    .await
    .unwrap();

    assert!(next_text(&mut ws).await.is_err());
    handle.await.unwrap();
}

/// Golden vector shared with `ControllerPairingTests.swift` on the iOS side — if this ever
/// fails, the two `compute_sas` implementations have drifted apart and pairing will show
/// different codes on each end.
#[test]
fn compute_sas_matches_the_swift_client_golden_vector() {
    let a = [1u8; 32];
    let b = [2u8; 32];
    let s = [3u8; 32];
    let fp = [4u8; 32];
    assert_eq!(compute_sas(&a, &b, &s, &fp), "061 920");
}
