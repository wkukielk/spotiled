use std::io;
use std::net::IpAddr;

pub struct MdnsService {
    _responder: libmdns::Responder,
    _service: libmdns::Service,
}

impl MdnsService {
    pub fn new(service_type: &str, instance_name: &str, port: u16) -> io::Result<Self> {
        let responder = libmdns::Responder::spawn_with_ip_list_and_hostname(
            &tokio::runtime::Handle::current(),
            advertised_ips()?,
            format!("{instance_name}-spotiled"),
        )?;
        let service = responder.register(service_type, instance_name, port, &[]);

        Ok(Self {
            _responder: responder,
            _service: service,
        })
    }
}

fn advertised_ips() -> io::Result<Vec<IpAddr>> {
    let ips: Vec<IpAddr> = if_addrs::get_if_addrs()?
        .into_iter()
        .filter(|iface| !iface.name.starts_with("lo") && iface.ip().is_ipv4())
        .map(|iface| iface.ip())
        .collect();

    if ips.is_empty() {
        return Err(io::Error::new(
            io::ErrorKind::NotFound,
            "no non-loopback IPv4 interfaces found; refusing to start mDNS unrestricted",
        ));
    }

    Ok(ips)
}
