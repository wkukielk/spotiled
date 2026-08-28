use super::*;
use wiremock::matchers::{method, path};
use wiremock::{Mock, MockServer, ResponseTemplate};

#[test]
fn encode_mercury_uri_percent_encodes_every_segment_but_the_first() {
    let encoded = Spotify::encode_mercury_uri(Spotify::HM_URL);

    assert_eq!(
        encoded,
        "hm://herodotus/uri/spotify%3Alist%3Aplay-history%3Av1/resume-point-revision"
    );
}

#[tokio::test]
async fn exchange_authorization_code_posts_grant_and_returns_credentials() {
    let server = MockServer::start().await;
    Mock::given(method("POST"))
        .and(path("/token"))
        .respond_with(ResponseTemplate::new(200).set_body_json(serde_json::json!({
            "access_token": "new-access-token",
            "token_type": "bearer",
            "expires_in": 3600,
            "refresh_token": "refresh-token",
        })))
        .mount(&server)
        .await;

    let token_url = format!("{}/token", server.uri());
    let credentials =
        Spotify::exchange_authorization_code("client-id", "auth-code", "verifier", &token_url)
            .await
            .unwrap();

    assert_eq!(
        credentials.auth_data,
        Credentials::with_access_token("new-access-token").auth_data
    );
}

#[tokio::test]
async fn exchange_authorization_code_returns_auth_exchange_error_on_server_rejection() {
    let server = MockServer::start().await;
    Mock::given(method("POST"))
        .and(path("/token"))
        .respond_with(ResponseTemplate::new(400).set_body_json(serde_json::json!({
            "error": "invalid_grant",
            "error_description": "Authorization code expired",
        })))
        .mount(&server)
        .await;

    let token_url = format!("{}/token", server.uri());
    let result =
        Spotify::exchange_authorization_code("client-id", "stale-code", "verifier", &token_url)
            .await;

    assert!(matches!(result, Err(SpotifyError::AuthExchange(_))));
}

#[tokio::test]
async fn exchange_authorization_code_returns_auth_exchange_error_on_invalid_token_url() {
    let result =
        Spotify::exchange_authorization_code("client-id", "auth-code", "verifier", "not-a-url")
            .await;

    assert!(matches!(result, Err(SpotifyError::AuthExchange(_))));
}
