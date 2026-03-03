#ifndef CONFIGURER_HPP
#define CONFIGURER_HPP

#include "BackendConfigurer.hpp"
#include "LogConfig.hpp"
#include "OTA.hpp"
#include "WiFiManager.hpp"
#include "WiFiManagerFactory.hpp"
#include "mDNSConfig.hpp"
#include "nvs.hpp"

struct Config {
    backend_config_t backend_cfg;
    u_ptr<I_WiFiManager> wifi_mgr;
    u_ptr<I_mDNS> mdns;
    u_ptr<I_FullOTA> ota;
};

class Configurer {
  public:
    explicit Configurer(I_NVS &cfgnvs, I_NVS &encnvs);
    void preconfigure() const;
    Config get_config() const;

  private:
    WiFiManagerFactory wifi_mgr_factory;
    mDNSConfig mdns_configurer;
    LogConfig log_configurer;
    BackendConfigurer backend_configurer;
};

#endif
