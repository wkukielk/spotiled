use super::*;
use mockall::predicate::eq;

#[test]
fn resolve_credentials_returns_cached_credentials_without_oauth() {
    let cached = Credentials::with_access_token("cached-token");
    let mut oauth = MockOAuthCredentialsProvider::new();
    oauth.expect_fetch().never();

    let resolved =
        Spotify::resolve_credentials(Some(cached.clone()), "client-id", &oauth).unwrap();

    assert_eq!(resolved.username, cached.username);
    assert_eq!(resolved.auth_data, cached.auth_data);
}

#[test]
fn resolve_credentials_falls_back_to_oauth_when_not_cached() {
    let oauth_credentials = Credentials::with_access_token("oauth-token");
    let mut oauth = MockOAuthCredentialsProvider::new();
    oauth
        .expect_fetch()
        .with(eq("client-id"))
        .times(1)
        .return_once(move |_| Ok(oauth_credentials));

    let resolved = Spotify::resolve_credentials(None, "client-id", &oauth).unwrap();

    assert_eq!(
        resolved.username,
        Credentials::with_access_token("oauth-token").username
    );
}

#[test]
fn resolve_credentials_propagates_oauth_error_when_not_cached() {
    let mut oauth = MockOAuthCredentialsProvider::new();
    oauth
        .expect_fetch()
        .times(1)
        .return_once(|_| Err(anyhow::anyhow!("oauth failed")));

    let result = Spotify::resolve_credentials(None, "client-id", &oauth);

    assert!(result.is_err());
}
