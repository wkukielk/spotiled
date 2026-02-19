#include "Configurer.hpp"
#include "WiFiManagerFactory.hpp"

Configurer::Configurer(I_NVS &cfgnvs)
    : cfgnvs{&cfgnvs}, wifi_mgr_factory{cfgnvs}, mdns_configurer{cfgnvs},
      log_configurer{cfgnvs}, backend_configurer{cfgnvs} {}

void Configurer::preconfigure() const { log_configurer.configure(); }

Config Configurer::get_config() const {
    return {.backend_cfg = backend_configurer.get(),
            .wifi_mgr = wifi_mgr_factory.get(),
            .mdns = mdns_configurer.get(),
            .ota = mkuptr<OTA>()};
}
