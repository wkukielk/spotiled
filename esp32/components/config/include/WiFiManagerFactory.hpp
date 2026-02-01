#ifndef WIFI_MANAGER_FACTORY_HPP
#define WIFI_MANAGER_FACTORY_HPP

#include "CredentialsWiFiManager.hpp"
#include "WiFiManager.hpp"
#include "errors.hpp"
#include "nvs.hpp"

struct ConfigError : public BaseError {
    ConfigError(const char *msg) : BaseError("ConfigError", msg){};
    ConfigError() : BaseError("ConfigError", ""){};
};

class WiFiManagerFactory {
  public:
    explicit WiFiManagerFactory(I_NVS &cfgnvs);
    u_ptr<I_WiFiManager> get() const;

  private:
    u_ptr<WiFiManager> get_wifi_manager() const;
    u_ptr<CredentialsWiFiManager> get_credentials_wifi_manager() const;
    Provisioning *get_provisioning(util::EventGroup &event_group) const;
    const I_NVS *cfgnvs{};
};

#endif
