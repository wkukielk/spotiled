#include "ConfigReader.hpp"
#include "ProvEventHandler.hpp"
#include "WiFiProv.hpp"

ConfigReader::ConfigReader(I_NVS &cfgnvs) : cfgnvs{&cfgnvs} {}

void ConfigReader::preconfigure() const {}

Config ConfigReader::get_config() const {
    auto event_group = mkuptr<util::EventGroup>();
    auto handler = mkuptr<WiFiEventHandler>(*event_group, 3);
    auto toggle_handler = mkuptr<ToggleEventHandler>(std::move(handler));
    auto provisioning = u_ptr<Provisioning>(get_provisioning(*event_group));
    return {.wifi_mgr = mkuptr<WiFiManager>(std::move(event_group),
                                            std::move(toggle_handler),
                                            std::move(provisioning))};
}

Provisioning *
ConfigReader::get_provisioning(util::EventGroup &event_group) const {
    ProvEssentials essentials = {
        .get_wifi_prov = []() { return new WiFiProv(&wifi_prov::cfg); },
        .get_handler =
            [](util::I_EventGroup &g) { return new ProvEventHandler{g, 5}; }};
    return new Provisioning{essentials, event_group, "MAC_HERE"};
}