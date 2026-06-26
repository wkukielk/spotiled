#ifndef WIFI_MANAGER_FACTORY_HPP
#define WIFI_MANAGER_FACTORY_HPP

#include "CredentialsWiFiManager.hpp"
#include "Handle.hpp"
#include "ProvEventHandler.hpp"
#include "Provisioning.hpp"
#include "WiFi.hpp"
#include "WiFiManager.hpp"
#include "WiFiProv.hpp"
#include "errors.hpp"
#include "nvs.hpp"

struct ConfigError : public BaseError {
    ConfigError(int reason, const char *msg)
        : BaseError("ConfigError", msg, reason) {};
    ConfigError() : BaseError("ConfigError", "") {};
};

struct DefaultWiFiEventHandlerTypes {
    using EventGroup = util::EventGroup;
    using EventLoop = Handle<DftEventLoop>;
    using WiFi = Handle<::WiFi>;
};
static_assert(WiFiEventHandlerTypes<DefaultWiFiEventHandlerTypes>);
using DftWiFiEventHandler = WiFiEventHandler<DefaultWiFiEventHandlerTypes>;

struct DftProvisioningTypes {
    using WiFiProv = ::WiFiProv;
    using EventGroup = ::util::EventGroup;
    using EventHandler = ProvEventHandler<EventGroup, Handle<DftEventLoop>>;
};
static_assert(ProvisioningTypes<DftProvisioningTypes>);
using DftProvisioning = Provisioning<DftProvisioningTypes>;

struct DefaultWiFiManagerTypes {
    using EventGroup = util::EventGroup;
    using EventHandler = DftWiFiEventHandler;
    using WiFi = Handle<::WiFi>;
    using Provisioning = DftProvisioning;
};
static_assert(WiFiManagerTypes<DefaultWiFiManagerTypes>);
using DftWiFiManager = WiFiManager<DefaultWiFiManagerTypes>;

struct DefaultCredentialsWiFiManagerTypes {
    using EventGroup = util::EventGroup;
    using EventHandler = DftWiFiEventHandler;
    using WiFi = Handle<::WiFi>;
};
static_assert(CredentialsWiFiManagerTypes<DefaultCredentialsWiFiManagerTypes>);
using DftCredentialsWiFiManager =
    CredentialsWiFiManager<DefaultCredentialsWiFiManagerTypes>;

class WiFiManagerFactory {
  public:
    explicit WiFiManagerFactory(I_NVS &cfgnvs);
    u_ptr<I_WiFiManager> get() const;

  private:
    u_ptr<DftWiFiManager> get_wifi_manager() const;
    u_ptr<DftCredentialsWiFiManager> get_credentials_wifi_manager() const;
    Provisioning<DftProvisioningTypes> *
    get_provisioning(util::EventGroup &event_group) const;
    const I_NVS *cfgnvs{};
};

#endif
