use anyhow::Result;
use futures_util::SinkExt;
use std::marker::PhantomData;
use tokio::net::TcpStream;
use tokio_tungstenite::{MaybeTlsStream, WebSocketStream, connect_async, tungstenite::Message};

use crate::dom_color::Rgb;

#[cfg_attr(test, mockall::automock)]
pub trait LedSocket: Sized + Send + Sync {
    async fn connect(url: &str) -> Result<Self>;
    fn send_rgb(&mut self, rgb: &Rgb) -> impl Future<Output = Result<(), std::io::Error>> + Send;
}

pub struct WsLedSocket {
    stream: WebSocketStream<MaybeTlsStream<TcpStream>>,
}

impl LedSocket for WsLedSocket {
    async fn connect(url: &str) -> Result<Self> {
        let (stream, _) = connect_async(url).await?;
        Ok(Self { stream })
    }

    async fn send_rgb(&mut self, rgb: &Rgb) -> Result<(), std::io::Error> {
        let msg = format!(
            r#"{{"cmd":"fade","r":{},"g":{},"b":{}}}"#,
            rgb.r, rgb.g, rgb.b
        );
        log::info!("sending {:?}", msg);
        self.stream
            .send(Message::text(msg))
            .await
            .map_err(|e| std::io::Error::new(std::io::ErrorKind::Other, e))?;
        Ok(())
    }
}

pub struct LogSocket {}

impl LedSocket for LogSocket {
    async fn connect(url: &str) -> Result<Self> {
        log::info!("connected to {}", url);
        Ok(Self {})
    }

    async fn send_rgb(&mut self, rgb: &Rgb) -> Result<(), std::io::Error> {
        let msg = format!(
            r#"{{"cmd":"fade","r":{},"g":{},"b":{}}}"#,
            rgb.r, rgb.g, rgb.b
        );
        log::info!("{}", msg);
        Ok(())
    }
}

pub struct LedSocketConnector<T: LedSocket> {
    pub url: String,
    pub _marker: PhantomData<T>,
}

impl<T: LedSocket> LedSocketConnector<T> {
    pub fn new(url: String) -> Self {
        Self {
            url,
            _marker: PhantomData,
        }
    }

    pub async fn connect(&self) -> Result<T> {
        T::connect(&self.url).await
    }
}
