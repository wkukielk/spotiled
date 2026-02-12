#include "ConfigReader.hpp"
#include "ProvEventHandler.hpp"
#include "WiFiManagerFactory.hpp"
#include "WiFiProv.hpp"
#include "data.hpp"
#include <cstring>

ConfigReader::ConfigReader(I_NVS &cfgnvs)
    : cfgnvs{&cfgnvs}, wifi_mgr_factory{cfgnvs}, mdns_configurer{cfgnvs} {}

void ConfigReader::preconfigure() const {}

Config ConfigReader::get_config() const {
    return {.wifi_mgr = wifi_mgr_factory.get(), .mdns = mdns_configurer.get()};
}

Provisioning *
WiFiManagerFactory::get_provisioning(util::EventGroup &event_group) const {
    ProvEssentials essentials = {
        .get_wifi_prov = []() { return new WiFiProv(&wifi_prov::cfg); },
        .get_handler =
            [](util::I_EventGroup &g) { return new ProvEventHandler{g, 5}; }};
    const auto name = cfgnvs->read(key::PROV_NAME, "spotiled_esp32");
    return new Provisioning{essentials, event_group, name.data()};
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

u_ptr<WiFiManager> WiFiManagerFactory::get_wifi_manager() const {
    auto event_group = mkuptr<util::EventGroup>();
    auto egrp = event_group.get();
    auto handler = [=]() { return mkuptr<WiFiEventHandler>(*egrp, 3); };
    auto toggle_handler = mkuptr<ToggleEventHandler>(handler);
    auto provisioning = u_ptr<Provisioning>(get_provisioning(*event_group));
    return mkuptr<WiFiManager>(std::move(event_group),
                               std::move(toggle_handler),
                               std::move(provisioning));
}

u_ptr<CredentialsWiFiManager>
WiFiManagerFactory::get_credentials_wifi_manager() const {
    auto event_group = mkuptr<util::EventGroup>();
    auto handler = mkuptr<WiFiEventHandler>(*event_group, 3);

    CharBuf ssid{}, passwd{};
    UTIL_THROW(cfgnvs->read(key::WIFI_SSID, ssid), ConfigError, "no ssid");
    UTIL_THROW(cfgnvs->read(key::WIFI_PASSWD, passwd), ConfigError,
               "no passwd");
    std::string_view ssid_v{ssid.data(), ssid.size()};
    std::string_view passwd_v{passwd.data(), passwd.size()};
    return mkuptr<CredentialsWiFiManager>(ssid_v, passwd_v, std::move(handler),
                                          std::move(event_group));
}
