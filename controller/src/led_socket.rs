use anyhow::Result;
use futures_util::SinkExt;
use tokio::net::TcpStream;
use tokio_tungstenite::{MaybeTlsStream, WebSocketStream, connect_async, tungstenite::Message};

use crate::dom_color::Rgb;

#[cfg_attr(test, mockall::automock)]
pub trait LedSocket: Send {
    async fn send_rgb(&mut self, rgb: &Rgb) -> Result<()>;
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
        self.stream
            .send(Message::binary(vec![rgb.r, rgb.g, rgb.b]))
            .await?;
        Ok(())
    }
}
