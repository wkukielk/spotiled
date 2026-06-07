#ifndef WIFI_MANAGER_FACTORY_HPP
#define WIFI_MANAGER_FACTORY_HPP

#include "CredentialsWiFiManager.hpp"
#include "WiFiManager.hpp"
#include "errors.hpp"
#include "WiFi.hpp"
#include "nvs.hpp"

struct ConfigError : public BaseError {
    ConfigError(int reason, const char *msg)
        : BaseError("ConfigError", msg, reason) {};
    ConfigError() : BaseError("ConfigError", "") {};
};

struct DefaultWiFiEventHandlerTypes {
    using EventGroup = util::EventGroup;
    using EventLoop = DftEventLoop;
    using WiFi = ::WiFi;
};

using DftWiFiEventHandler = WiFiEventHandler<DefaultWiFiEventHandlerTypes>;
struct DefaultWiFiManagerTypes {
    using EventGroup = util::EventGroup;
    using EventHandler = DftWiFiEventHandler;
    using WiFi = ::WiFi;
};

using DftCredentialsWiFiManager =
    CredentialsWiFiManager<DefaultWiFiManagerTypes>;

using DftWiFiManager = WiFiManager<DefaultWiFiManagerTypes>;

class WiFiManagerFactory {
  public:
    explicit WiFiManagerFactory(I_NVS &cfgnvs);
    u_ptr<I_WiFiManager> get() const;

  private:
    u_ptr<DftWiFiManager> get_wifi_manager() const;
    u_ptr<DftCredentialsWiFiManager> get_credentials_wifi_manager() const;
    Provisioning *get_provisioning(util::EventGroup &event_group) const;
    const I_NVS *cfgnvs{};
};

#endif
