use std::time::Duration;

use anyhow::{Context, Result, bail};
use base64::Engine;
use base64::engine::general_purpose::STANDARD as BASE64;
use futures_util::{SinkExt, StreamExt};
use serde::Deserialize;
use sha2::{Digest, Sha256};
use tokio::io::{AsyncRead, AsyncWrite};
use tokio::time::timeout;
use tokio_tungstenite::WebSocketStream;
use tokio_tungstenite::tungstenite::Message as WsMessage;
use x25519_dalek::{EphemeralSecret, PublicKey};

use crate::status::{self, ControllerStatus};

const HANDSHAKE_TIMEOUT: Duration = Duration::from_secs(30);
const CONFIRM_TIMEOUT: Duration = Duration::from_secs(60);

pub fn compute_sas(
    client_pub: &[u8; 32],
    server_pub: &[u8; 32],
    shared: &[u8; 32],
    cert_fp: &[u8; 32],
) -> String {
    let mut hasher = Sha256::new();
    hasher.update(client_pub);
    hasher.update(server_pub);
    hasher.update(shared);
    hasher.update(cert_fp);
    let digest = hasher.finalize();
    let code = u32::from_be_bytes([digest[0], digest[1], digest[2], digest[3]]) % 1_000_000;
    format!("{:03} {:03}", code / 1000, code % 1000)
}

#[derive(Deserialize)]
#[serde(tag = "type", rename_all = "snake_case")]
enum PairingMessage {
    Commit {
        hash: String,
    },
    Reveal {
        #[serde(rename = "pub")]
        public: String,
    },
    Confirm,
    Reject,
}

pub async fn run_pairing<S>(mut ws: WebSocketStream<S>, cert_fp: [u8; 32])
where
    S: AsyncRead + AsyncWrite + Unpin,
{
    if let Err(err) = run_pairing_inner(&mut ws, cert_fp).await {
        log::warn!("pairing attempt failed: {err:#}");
    }
    status::set(ControllerStatus::Running);
    let _ = ws.close(None).await;
}

async fn run_pairing_inner<S>(ws: &mut WebSocketStream<S>, cert_fp: [u8; 32]) -> Result<()>
where
    S: AsyncRead + AsyncWrite + Unpin,
{
    let client_commit = timeout(HANDSHAKE_TIMEOUT, recv_commit(ws))
        .await
        .context("timed out waiting for client commit")??;

    let secret = EphemeralSecret::random();
    let server_pub = PublicKey::from(&secret);
    send_json(
        ws,
        &serde_json::json!({ "type": "reveal", "pub": BASE64.encode(server_pub.as_bytes()) }),
    )
    .await?;

    let client_pub_bytes = timeout(HANDSHAKE_TIMEOUT, recv_reveal(ws))
        .await
        .context("timed out waiting for client reveal")??;
    let actual_client_commit: [u8; 32] = Sha256::digest(client_pub_bytes).into();
    if actual_client_commit != client_commit {
        bail!("client's revealed key does not match its earlier commitment");
    }

    let client_pub = PublicKey::from(client_pub_bytes);
    let shared = secret.diffie_hellman(&client_pub);

    let sas = compute_sas(
        &client_pub_bytes,
        server_pub.as_bytes(),
        shared.as_bytes(),
        &cert_fp,
    );
    log::info!("Pairing requested. Verification code: {sas}");
    status::set(ControllerStatus::Pairing { sas });

    let confirmed = timeout(CONFIRM_TIMEOUT, recv_confirm(ws))
        .await
        .context("timed out waiting for pairing confirmation")??;

    if confirmed {
        send_json(ws, &serde_json::json!({ "type": "paired" })).await?;
        log::info!("Pairing confirmed.");
    } else {
        send_json(ws, &serde_json::json!({ "type": "rejected" })).await?;
        log::info!("Pairing rejected by client.");
    }
    Ok(())
}

async fn next_text<S>(ws: &mut WebSocketStream<S>) -> Result<String>
where
    S: AsyncRead + AsyncWrite + Unpin,
{
    loop {
        match ws.next().await {
            Some(Ok(WsMessage::Text(text))) => return Ok(text.to_string()),
            Some(Ok(WsMessage::Close(_))) | None => bail!("connection closed"),
            Some(Ok(_)) => continue,
            Some(Err(err)) => bail!("websocket error: {err}"),
        }
    }
}

async fn recv_commit<S>(ws: &mut WebSocketStream<S>) -> Result<[u8; 32]>
where
    S: AsyncRead + AsyncWrite + Unpin,
{
    let text = next_text(ws).await?;
    match serde_json::from_str(&text).context("malformed pairing message")? {
        PairingMessage::Commit { hash } => {
            let bytes = BASE64
                .decode(hash)
                .context("commit hash is not valid base64")?;
            bytes
                .try_into()
                .map_err(|_| anyhow::anyhow!("commit hash is not 32 bytes"))
        }
        _ => bail!("expected commit, got a different pairing message"),
    }
}

async fn recv_reveal<S>(ws: &mut WebSocketStream<S>) -> Result<[u8; 32]>
where
    S: AsyncRead + AsyncWrite + Unpin,
{
    let text = next_text(ws).await?;
    match serde_json::from_str(&text).context("malformed pairing message")? {
        PairingMessage::Reveal { public } => {
            let bytes = BASE64
                .decode(public)
                .context("revealed public key is not valid base64")?;
            bytes
                .try_into()
                .map_err(|_| anyhow::anyhow!("revealed public key is not 32 bytes"))
        }
        _ => bail!("expected reveal, got a different pairing message"),
    }
}

async fn recv_confirm<S>(ws: &mut WebSocketStream<S>) -> Result<bool>
where
    S: AsyncRead + AsyncWrite + Unpin,
{
    let text = next_text(ws).await?;
    match serde_json::from_str(&text).context("malformed pairing message")? {
        PairingMessage::Confirm => Ok(true),
        PairingMessage::Reject => Ok(false),
        _ => bail!("expected confirm/reject, got a different pairing message"),
    }
}

async fn send_json<S>(ws: &mut WebSocketStream<S>, value: &serde_json::Value) -> Result<()>
where
    S: AsyncRead + AsyncWrite + Unpin,
{
    ws.send(WsMessage::text(value.to_string()))
        .await
        .context("failed to send pairing message")
}

#[cfg(test)]
mod pairing_tests;
