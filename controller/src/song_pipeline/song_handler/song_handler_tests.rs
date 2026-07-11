use super::*;
use crate::dom_color::{MockJPEGDecoder, Rgb};
use crate::led_socket::MockLedSocket;
use crate::spotify::{AudioItem, MockISpotify, SpotifyError};

use librespot::core::dealer::protocol::{Message, PayloadValue};
use librespot::metadata::audio::item::CoverImage;
use librespot::metadata::image::ImageSize;
use mockall::predicate::eq;
use std::sync::Arc;
use wiremock::matchers::{method, path};
use wiremock::{Mock, MockServer, ResponseTemplate};

pub fn raw_message(uri: &str) -> Result<Message, SpotifyError> {
    Ok(Message {
        headers: Default::default(),
        payload: PayloadValue::Raw(format!("junk {} junk", uri).into_bytes()),
        uri: String::new(),
    })
}

pub fn cover(url: String) -> CoverImage {
    CoverImage {
        url,
        size: ImageSize::DEFAULT,
        width: 2,
        height: 2,
    }
}

#[test]
fn extract_track_uri_finds_uri_within_surrounding_bytes() {
    let bytes = b"garbage spotify:track:abc123XYZ trailing garbage";

    let uri = DftCoverFetcher::extract_track_uri(bytes);

    assert_eq!(uri.as_deref(), Some("spotify:track:abc123XYZ"));
}

#[test]
fn extract_track_uri_returns_none_without_a_track_uri() {
    let uri = DftCoverFetcher::extract_track_uri(b"no track uri here");

    assert!(uri.is_none());
}

#[test]
fn extract_cover_uri_returns_uri_from_raw_payload() {
    let uri = DftCoverFetcher::extract_cover_uri(raw_message("spotify:track:abc123")).unwrap();

    assert_eq!(uri, "spotify:track:abc123");
}

#[test]
fn extract_cover_uri_errors_on_non_raw_payload() {
    let message = Ok(Message {
        headers: Default::default(),
        payload: PayloadValue::Json("{}".to_string()),
        uri: String::new(),
    });

    let result = DftCoverFetcher::extract_cover_uri(message);

    assert!(matches!(
        result,
        Err(SongPipelineError::UnexpectedDealerMessage)
    ));
}

#[test]
fn extract_cover_uri_propagates_dealer_error() {
    let result = DftCoverFetcher::extract_cover_uri(Err(SpotifyError::NotInit));

    assert!(matches!(result, Err(SongPipelineError::SpotifyError(_))));
}

#[tokio::test]
async fn fetch_returns_cover_bytes_on_success() {
    let server = MockServer::start().await;
    Mock::given(method("GET"))
        .and(path("/cover.jpg"))
        .respond_with(ResponseTemplate::new(200).set_body_bytes(b"jpeg-bytes".to_vec()))
        .mount(&server)
        .await;
    let cover_url = format!("{}/cover.jpg", server.uri());

    let mut spotify = MockISpotify::new();
    spotify
        .expect_get_audio_item()
        .withf(|uri| uri == "spotify:track:abc123")
        .times(1)
        .return_once(move |_| {
            Ok(AudioItem {
                covers: vec![cover(cover_url)],
            })
        });

    let fetcher = DftCoverFetcher::new(Arc::new(spotify));

    let bytes = fetcher
        .fetch(raw_message("spotify:track:abc123"))
        .await
        .unwrap();

    assert_eq!(bytes.as_ref(), b"jpeg-bytes");
}

#[tokio::test]
async fn fetch_errors_when_track_has_no_cover() {
    let mut spotify = MockISpotify::new();
    spotify
        .expect_get_audio_item()
        .times(1)
        .return_once(|_| Ok(AudioItem { covers: vec![] }));

    let fetcher = DftCoverFetcher::new(Arc::new(spotify));

    let result = fetcher.fetch(raw_message("spotify:track:abc123")).await;

    assert!(matches!(result, Err(SongPipelineError::NoCover)));
}

#[tokio::test]
async fn fetch_errors_when_spotify_lookup_fails() {
    let mut spotify = MockISpotify::new();
    spotify
        .expect_get_audio_item()
        .times(1)
        .return_once(|_| Err(SpotifyError::NotInit));

    let fetcher = DftCoverFetcher::new(Arc::new(spotify));

    let result = fetcher.fetch(raw_message("spotify:track:abc123")).await;

    assert!(matches!(result, Err(SongPipelineError::SpotifyError(_))));
}

#[tokio::test]
async fn fetch_errors_when_cover_download_fails() {
    let server = MockServer::start().await;
    Mock::given(method("GET"))
        .and(path("/missing.jpg"))
        .respond_with(ResponseTemplate::new(404))
        .mount(&server)
        .await;
    let cover_url = format!("{}/missing.jpg", server.uri());

    let mut spotify = MockISpotify::new();
    spotify
        .expect_get_audio_item()
        .times(1)
        .return_once(move |_| {
            Ok(AudioItem {
                covers: vec![cover(cover_url)],
            })
        });

    let fetcher = DftCoverFetcher::new(Arc::new(spotify));

    let result = fetcher.fetch(raw_message("spotify:track:abc123")).await;

    assert!(matches!(result, Err(SongPipelineError::HTTPClientError(_))));
}

#[tokio::test]
async fn fetch_errors_on_malformed_dealer_message() {
    let spotify = MockISpotify::new();
    let fetcher = DftCoverFetcher::new(Arc::new(spotify));
    let message = Ok(Message {
        headers: Default::default(),
        payload: PayloadValue::Empty,
        uri: String::new(),
    });

    let result = fetcher.fetch(message).await;

    assert!(matches!(
        result,
        Err(SongPipelineError::UnexpectedDealerMessage)
    ));
}

fn solid_rgb565_pixels(r: u8, g: u8, b: u8) -> Vec<u16> {
    let pixel = ((r as u16 & 0xf8) << 8) | ((g as u16 & 0xfc) << 3) | (b as u16 >> 3);
    vec![pixel; 4]
}

fn tiny_jpeg() -> bytes::Bytes {
    let image = image::RgbImage::from_pixel(2, 2, image::Rgb([10, 20, 30]));
    let mut jpg = Vec::new();
    image::codecs::jpeg::JpegEncoder::new(&mut jpg)
        .encode_image(&image)
        .unwrap();
    bytes::Bytes::from(jpg)
}

#[tokio::test]
async fn test_song_handler() {
    let spotify = MockISpotify::new();
    let mut socket = MockLedSocket::new();
    socket
        .expect_send_rgb()
        .with(eq(Rgb { r: 8, g: 20, b: 24 }))
        .times(1)
        .returning(|_| Box::pin(async { Ok(()) }));
    let new_ctx = MockCoverFetcher::new_context();
    new_ctx.expect().returning(|_| MockCoverFetcher::default());
    let decode_ctx = MockJPEGDecoder::decode_jpg_to_rgb565_context();
    decode_ctx
        .expect()
        .returning(|_| Ok(solid_rgb565_pixels(10, 20, 30)));
    let mut handler = SongHandler::<MockCoverFetcher, MockLedSocket, MockJPEGDecoder>::new(
        Arc::new(spotify),
        socket,
    );

    let PayloadValue::Raw(exp_bytes) = raw_message("message").unwrap().payload else {
        unreachable!()
    };
    handler
        .cover_fetcher
        .expect_fetch()
        .withf(move |message| {
            matches!(
                message,
                Ok(Message { payload: PayloadValue::Raw(bytes), .. }) if *bytes == exp_bytes
            )
        })
        .times(1)
        .return_once(|_| Box::pin(async { Ok(tiny_jpeg()) }));
    let result = handler.on_song_changed_impl(raw_message("message")).await;
    assert!(matches!(result, Ok(())))
}
