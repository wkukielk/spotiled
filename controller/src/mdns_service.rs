use std::io;

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
        let responder = libmdns::Responder::spawn(&tokio::runtime::Handle::current())?;
        let service = responder.register(service_type, instance_name, port, &[]);

        Ok(Self {
            _responder: responder,
            _service: service,
        })
    }
}
