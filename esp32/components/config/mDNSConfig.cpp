#include "mDNSConfig.hpp"
#include "PersistentMDNS.hpp"
#include "data.hpp"
#include "errors.hpp"

mDNSConfig::mDNSConfig(I_NVS &cfgnvs, I_NVS &nvs)
    : cfgnvs{&cfgnvs}, nvs{&nvs} {}

u_ptr<I_mDNSFacade> mDNSConfig::get() const {
    auto hostname = cfgnvs->read(key::MDNS_HOSTNAME);

    auto default_instance = cfgnvs->read(key::MDNS_INSTANCE);
    auto instance = nvs->read(key::MDNS_INSTANCE, default_instance);

    auto mdns = mkuptr<PersistentMDNS<mDNS>>(*nvs, key::MDNS_INSTANCE);
    mdns->mDNS::set_hostname(hostname.data());
    mdns->mDNS::set_instance_name(instance.data());

    auto txt = std::span<mdns::txt_item_t>{};
    mdns->add_service(nullptr, "_s-led-auto", "_tcp", 8000, txt);
    mdns->add_service(nullptr, "_s-led-direct", "_udp", 8001, txt);
    return mdns;
}
