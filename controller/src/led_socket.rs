use anyhow::Result;
use futures_util::SinkExt;
use tokio::net::TcpStream;
use tokio_tungstenite::{MaybeTlsStream, WebSocketStream, connect_async, tungstenite::Message};

use crate::dom_color::Rgb;

#[cfg_attr(test, mockall::automock)]
pub trait LedSocket: Send + Sync {
    fn send_rgb(&mut self, rgb: &Rgb) -> impl Future<Output = Result<()>> + Send;
}

pub struct WsLedSocket {
    stream: WebSocketStream<MaybeTlsStream<TcpStream>>,
}

impl WsLedSocket {
    pub async fn connect(url: &str) -> Result<Self> {
        let (stream, _) = connect_async(url).await?;
        Ok(Self { stream })
    }
}

impl LedSocket for WsLedSocket {
    async fn send_rgb(&mut self, rgb: &Rgb) -> Result<()> {
        let msg = format!(
            r#"{{"cmd":"fade","r":{},"g":{},"b":{}}}"#,
            rgb.r, rgb.g, rgb.b
        );
        self.stream.send(Message::text(msg)).await?;
        Ok(())
    }
}

pub trait LedSocketConnector: Send + Sync {
    type Socket: LedSocket + 'static;

    fn connect(&self) -> impl Future<Output = Result<Self::Socket>> + Send;
}

pub struct WsLedSocketConnector {
    pub url: String,
}

impl LedSocketConnector for WsLedSocketConnector {
    type Socket = WsLedSocket;

    async fn connect(&self) -> Result<WsLedSocket> {
        WsLedSocket::connect(&self.url).await
    }
}
