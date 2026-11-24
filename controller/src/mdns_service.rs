use std::io;
use std::net::IpAddr;
use std::net::UdpSocket;

pub struct MdnsService {
    _responder: libmdns::Responder,
    _service: libmdns::Service,
}

impl MdnsService {
    pub fn new(service_type: &str, instance_name: &str, port: u16) -> io::Result<Self> {
        let responder = libmdns::Responder::spawn_with_ip_list(
            &tokio::runtime::Handle::current(),
            vec![advertised_ip()?],
        )?;
        let service = responder.register(service_type, instance_name, port, &[]);

        Ok(Self {
            _responder: responder,
            _service: service,
        })
    }
}

fn advertised_ip() -> io::Result<IpAddr> {
    let sock = UdpSocket::bind("0.0.0.0:0")?;
    sock.connect("8.8.8.8:53")?;
    Ok(sock.local_addr()?.ip())
}
