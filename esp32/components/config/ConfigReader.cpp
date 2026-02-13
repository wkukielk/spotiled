#include "ConfigReader.hpp"
#include "WiFiManagerFactory.hpp"

ConfigReader::ConfigReader(I_NVS &cfgnvs)
    : cfgnvs{&cfgnvs}, wifi_mgr_factory{cfgnvs}, mdns_configurer{cfgnvs},
      log_configurer{cfgnvs} {}

void ConfigReader::preconfigure() const { log_configurer.configure(); }

Config ConfigReader::get_config() const {
    return {.wifi_mgr = wifi_mgr_factory.get(), .mdns = mdns_configurer.get()};
}
