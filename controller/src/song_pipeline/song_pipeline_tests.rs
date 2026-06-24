use std::collections::HashMap;

use librespot::core::{SpotifyId, SpotifyUri};
use librespot::metadata::artist::ArtistsWithRole;
use librespot::metadata::audio::{AudioFiles, UniqueFields};
use librespot::metadata::audio::item::CoverImage;
use librespot::metadata::image::ImageSize;
use wiremock::matchers::{method, path};
use wiremock::{Mock, MockServer, ResponseTemplate};

use super::*;
use crate::led_socket::MockLedSocket;

fn extractor() -> CoverColorExtractor<MockLedSocket> {
    let socket = Arc::new(Mutex::new(MockLedSocket::new()));
    CoverColorExtractor::new(socket)
}

fn extractor_with(socket: MockLedSocket) -> CoverColorExtractor<MockLedSocket> {
    CoverColorExtractor::new(Arc::new(Mutex::new(socket)))
}

fn test_audio_item(covers: Vec<CoverImage>) -> AudioItem {
    AudioItem {
        track_id: SpotifyUri::Track {
            id: SpotifyId { id: 0 },
        },
        uri: "spotify:track:test".to_string(),
        files: AudioFiles(HashMap::new()),
        name: "Test Song".to_string(),
        covers,
        language: vec![],
        duration_ms: 0,
        is_explicit: false,
        availability: Ok(()),
        alternatives: None,
        unique_fields: UniqueFields::Track {
            artists: ArtistsWithRole(vec![]),
            album: String::new(),
            album_artists: vec![],
            popularity: 0,
            number: 0,
            disc_number: 0,
        },
    }
}

fn tiny_jpeg() -> Vec<u8> {
    let image = image::RgbImage::from_pixel(2, 2, image::Rgb([10, 20, 30]));
    let mut jpg = Vec::new();
    image::codecs::jpeg::JpegEncoder::new(&mut jpg)
        .encode_image(&image)
        .unwrap();
    jpg
}

#[tokio::test]
async fn fetch_cover_returns_body_bytes_on_success() {
    let server = MockServer::start().await;
    Mock::given(method("GET"))
        .and(path("/cover.jpg"))
        .respond_with(ResponseTemplate::new(200).set_body_bytes(b"jpeg-bytes".to_vec()))
        .mount(&server)
        .await;

    let extractor = extractor();
    let url = format!("{}/cover.jpg", server.uri());

    let bytes = extractor.fetch_cover(&url).await.unwrap();

    assert_eq!(bytes.as_ref(), b"jpeg-bytes");
}

#[tokio::test]
async fn fetch_cover_errors_on_http_error_status() {
    let server = MockServer::start().await;
    Mock::given(method("GET"))
        .and(path("/missing.jpg"))
        .respond_with(ResponseTemplate::new(404))
        .mount(&server)
        .await;

    let extractor = extractor();
    let url = format!("{}/missing.jpg", server.uri());

    let result = extractor.fetch_cover(&url).await;

    assert!(result.is_err());
}

#[tokio::test]
async fn on_song_changed_impl_sends_dominant_color_when_cover_present() {
    let server = MockServer::start().await;
    Mock::given(method("GET"))
        .and(path("/cover.jpg"))
        .respond_with(ResponseTemplate::new(200).set_body_bytes(tiny_jpeg()))
        .mount(&server)
        .await;

    let mut socket = MockLedSocket::new();
    socket.expect_send_rgb().times(1).returning(|_| Ok(()));
    let extractor = extractor_with(socket);

    let audio_item = test_audio_item(vec![CoverImage {
        url: format!("{}/cover.jpg", server.uri()),
        size: ImageSize::DEFAULT,
        width: 2,
        height: 2,
    }]);

    let result = extractor.on_song_changed_impl(audio_item).await;

    assert!(result.is_ok());
}

#[tokio::test]
async fn on_song_changed_impl_errors_without_cover() {
    let extractor = extractor();
    let audio_item = test_audio_item(vec![]);

    let result = extractor.on_song_changed_impl(audio_item).await;

    assert!(result.is_err());
}
