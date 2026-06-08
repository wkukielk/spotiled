#include "WiFiManagerFactory.hpp"
#include "ProvEventHandler.hpp"
#include "WiFiProv.hpp"
#include "data.hpp"
#include "nvs.hpp"
#include <cstring>

Provisioning<DftProvisioningTypes> *
WiFiManagerFactory::get_provisioning(util::EventGroup &event_group) const {
    auto name = cfgnvs->read(key::PROV_NAME);
    return new Provisioning<DftProvisioningTypes>{event_group, name.data()};
}

WiFiManagerFactory::WiFiManagerFactory(I_NVS &cfgnvs) : cfgnvs{&cfgnvs} {}

u_ptr<I_WiFiManager> WiFiManagerFactory::get() const {
    auto event_group = mkuptr<util::EventGroup>();

    auto doProvision = cfgnvs->read(key::PROVISIONING, B::yes);
    if (strcmp(doProvision.data(), B::yes) == 0)
        return get_wifi_manager();
    else
        return get_credentials_wifi_manager();
}

u_ptr<DftWiFiManager> WiFiManagerFactory::get_wifi_manager() const {
    auto event_group = mkuptr<util::EventGroup>();
    auto get_handler = [](auto &egroup) {
        return WiFiEventHandler<DefaultWiFiEventHandlerTypes>{egroup, 3};
    };
    return mkuptr<DftWiFiManager>(get_handler);
}

u_ptr<DftCredentialsWiFiManager>
WiFiManagerFactory::get_credentials_wifi_manager() const {
    auto ssid = cfgnvs->read(key::WIFI_SSID);
    auto passwd = cfgnvs->read(key::WIFI_PASSWD);
    auto ssid_v = std::string_view{ssid.data(), ssid.size()};
    auto passwd_v = std::string_view{passwd.data(), passwd.size()};
    return mkuptr<DftCredentialsWiFiManager>(ssid_v, passwd_v);
}
