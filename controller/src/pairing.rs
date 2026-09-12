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

const HANDSHAKE_TIMEOUT: Duration = Duration::from_secs(30);
const CONFIRM_TIMEOUT: Duration = Duration::from_secs(60);

/// SAS = SHA256(client_pub || server_pub || shared_secret || cert_fp), first 4 bytes as a
/// big-endian u32 mod 1_000_000, zero-padded to 6 digits, grouped "123 456".
///
/// `cert_fp` is never exchanged as a `/pair` message field — this whole exchange already happens
/// *inside* the TLS connection, so both sides already know which certificate is in play: the
/// server from its own identity, the client from the certificate its own TLS stack presented
/// during the handshake it just completed. Sending it as JSON would add nothing (a relaying MITM
/// running two separate TLS legs could just forward the server's honest value straight through)
/// and inviting the client to trust an in-band claim instead of its own TLS-layer observation is
/// exactly the mistake that would let such a relay through undetected.
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

/// Handles one `/pair` connection end to end: a client-commits/server-replies/client-reveals
/// ephemeral X25519 handshake, SAS logging, then waits for the client's confirm/reject. Runs
/// independently of the main command connection and never touches it.
///
/// Only the client commits (`SHA256(c_pub)`) before revealing its real key — that alone is
/// enough: we only ever generate our own ephemeral key *after* seeing the client's commitment
/// hash, never the raw value, so our choice can't be influenced by (or grind against) theirs; and
/// the client, having already committed before it ever sees our key, can't retroactively pick a
/// different one either. A grinding attacker would need ~2^20 tries (cheap, offline-fast) to
/// force a chosen SAS if either side could choose its key after seeing the other's — this closes
/// that off for both sides with a single one-way commitment.
pub async fn run_pairing<S>(mut ws: WebSocketStream<S>, cert_fp: [u8; 32])
where
    S: AsyncRead + AsyncWrite + Unpin,
{
    if let Err(err) = run_pairing_inner(&mut ws, cert_fp).await {
        log::warn!("pairing attempt failed: {err:#}");
    }
    let _ = ws.close(None).await;
}

async fn run_pairing_inner<S>(ws: &mut WebSocketStream<S>, cert_fp: [u8; 32]) -> Result<()>
where
    S: AsyncRead + AsyncWrite + Unpin,
{
    // The client commits first; we only ever see the hash at this point, never the raw key.
    let client_commit = timeout(HANDSHAKE_TIMEOUT, recv_commit(ws))
        .await
        .context("timed out waiting for client commit")??;

    // We generate our own ephemeral key having seen nothing but that commitment hash, so we
    // can send it straight away — there's no need for us to also commit-then-reveal.
    let secret = EphemeralSecret::random();
    let server_pub = PublicKey::from(&secret);
    send_json(
        ws,
        &serde_json::json!({ "type": "reveal", "pub": BASE64.encode(server_pub.as_bytes()) }),
    )
    .await?;

    // Now that the client has seen our key, it reveals its own — verify it against the
    // commitment it sent before it could have known ours.
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
