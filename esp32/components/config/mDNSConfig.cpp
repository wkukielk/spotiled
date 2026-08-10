#include "mDNSConfig.hpp"
#include "data.hpp"
#include "errors.hpp"

mDNSConfig::mDNSConfig(I_NVS &cfgnvs) : cfgnvs{&cfgnvs} {}

u_ptr<I_mDNS> mDNSConfig::get() const {
    auto mdns = mkuptr<mDNS>();
    auto hostname = cfgnvs->read(key::MDNS_HOSTNAME);
    auto instance = cfgnvs->read(key::MDNS_INSTANCE);

    mdns->set_hostname(hostname.data());
    auto txt = std::span<mdns::txt_item_t>{};
    mdns->add_service(nullptr, "_s-led-auto", "_tcp", 8000, txt);
    mdns->add_service(nullptr, "_s-led-direct", "_udp", 8001, txt);
    return mdns;
}
