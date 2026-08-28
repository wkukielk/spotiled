use std::io;
use std::net::IpAddr;

/// Advertises a service over mDNS/DNS-SD for as long as it is kept alive.
/// The advertisement is withdrawn when this value is dropped.
///
/// Must be constructed from within a running Tokio runtime.
pub struct MdnsService {
    _responder: libmdns::Responder,
    _service: libmdns::Service,
}

impl MdnsService {
    /// `service_type` should be in the form `_service._tcp` (e.g. `_ws._tcp`).
    pub fn new(service_type: &str, instance_name: &str, port: u16) -> io::Result<Self> {
        let responder = libmdns::Responder::spawn_with_ip_list(
            &tokio::runtime::Handle::current(),
            advertised_ips()?,
        )?;
        let service = responder.register(service_type, instance_name, port, &[]);

        Ok(Self {
            _responder: responder,
            _service: service,
        })
    }
}

/// Restricted to non-loopback IPv4 addresses. The controller is meant to run on arbitrary
/// end-user PCs, which routinely have VPN/virtualization/other adapters carrying their own
/// link-local IPv6 addresses that aren't reachable from any other device on the network. Unlike
/// IPv4, an IPv6 link-local address is only valid on the specific interface it's scoped to, so a
/// client that resolves to the wrong candidate doesn't just fail to connect — the OS reports the
/// path as routable and it hangs until it times out. Plain LAN IPv4 addresses don't have that
/// ambiguity, so advertising only those avoids the whole class of failure.
///
/// (Also excludes loopback: `libmdns`'s own loopback filtering only recognizes the literal
/// `::1`/`127.0.0.1` addresses, so on macOS — where `lo0` also carries the link-local address
/// `fe80::1` — it would otherwise advertise that alongside the real one.)
fn advertised_ips() -> io::Result<Vec<IpAddr>> {
    let ips: Vec<IpAddr> = if_addrs::get_if_addrs()?
        .into_iter()
        .filter(|iface| !iface.name.starts_with("lo") && iface.ip().is_ipv4())
        .map(|iface| iface.ip())
        .collect();

    // libmdns treats an *empty* allow-list as "no restriction" rather than "advertise
    // nothing" (see `add_ip_rr` in libmdns's fsm.rs), so silently passing one through here
    // would defeat the IPv4-only filtering below and let it fall back to advertising every
    // interface, IPv6 link-locals included. This list is captured once at startup, so that
    // state would then persist for the life of the process. Fail loudly instead so a
    // transient no-IPv4-yet moment (e.g. Wi-Fi still associating) doesn't get silently
    // upgraded into "no restriction, ever" once mDNS starts advertising.
    if ips.is_empty() {
        return Err(io::Error::new(
            io::ErrorKind::NotFound,
            "no non-loopback IPv4 interfaces found; refusing to start mDNS unrestricted",
        ));
    }

    Ok(ips)
}
