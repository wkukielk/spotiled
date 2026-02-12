#include "mDNSConfig.hpp"
#include "data.hpp"
#include "errors.hpp"

mDNSConfig::mDNSConfig(I_NVS &cfgnvs) : cfgnvs{&cfgnvs} {}

u_ptr<I_mDNS> mDNSConfig::get() const {
    auto mdns = mkuptr<mDNS>();
    auto hostname = cfgnvs->read(key::MDNS_HOSTNAME);
    auto instance = cfgnvs->read(key::MDNS_INSTANCE);

    mdns->set_hostname(hostname.data());
    mdns->set_instance_name(instance.data());
    return mdns;
}
