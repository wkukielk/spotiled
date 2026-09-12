use std::fs;
use std::path::Path;
use std::sync::Arc;

use anyhow::{Context, Result};
use rcgen::{CertifiedKey, generate_simple_self_signed};
use rustls::ServerConfig;
use rustls::pki_types::{CertificateDer, PrivateKeyDer};
use sha2::{Digest, Sha256};
use tokio_rustls::TlsAcceptor;

const CERT_FILE: &str = "cert.pem";
const KEY_FILE: &str = "key.pem";

/// Loads the controller's TLS identity from `dir`, generating and persisting a new self-signed
/// one on first run. The returned fingerprint is the SHA-256 of the leaf certificate's DER bytes;
/// it stays stable across restarts since the identity is persisted, and is what `pairing` folds
/// into the SAS and what clients pin after a successful pairing.
pub fn load_or_generate(dir: &Path, subject_name: &str) -> Result<(TlsAcceptor, [u8; 32])> {
    let cert_path = dir.join(CERT_FILE);
    let key_path = dir.join(KEY_FILE);

    let (cert_pem, key_pem) = if cert_path.exists() && key_path.exists() {
        (
            fs::read_to_string(&cert_path)
                .with_context(|| format!("failed to read {}", cert_path.display()))?,
            fs::read_to_string(&key_path)
                .with_context(|| format!("failed to read {}", key_path.display()))?,
        )
    } else {
        fs::create_dir_all(dir).with_context(|| format!("failed to create {}", dir.display()))?;
        let CertifiedKey { cert, signing_key } =
            generate_simple_self_signed(vec![subject_name.to_string()])
                .context("failed to generate self-signed certificate")?;
        let cert_pem = cert.pem();
        let key_pem = signing_key.serialize_pem();
        fs::write(&cert_path, &cert_pem)
            .with_context(|| format!("failed to write {}", cert_path.display()))?;
        fs::write(&key_path, &key_pem)
            .with_context(|| format!("failed to write {}", key_path.display()))?;
        log::info!("Generated new controller TLS identity at {}", dir.display());
        (cert_pem, key_pem)
    };

    let certs: Vec<CertificateDer<'static>> = rustls_pemfile::certs(&mut cert_pem.as_bytes())
        .collect::<Result<_, _>>()
        .context("failed to parse certificate PEM")?;
    let leaf = certs.first().context("cert.pem contains no certificate")?;
    let fingerprint: [u8; 32] = Sha256::digest(leaf.as_ref()).into();

    let key: PrivateKeyDer<'static> = rustls_pemfile::private_key(&mut key_pem.as_bytes())
        .context("failed to parse private key PEM")?
        .context("key.pem contains no private key")?;

    let config = ServerConfig::builder()
        .with_no_client_auth()
        .with_single_cert(certs, key)
        .context("failed to build TLS server config")?;

    Ok((TlsAcceptor::from(Arc::new(config)), fingerprint))
}

#[cfg(test)]
mod tls_identity_tests;
