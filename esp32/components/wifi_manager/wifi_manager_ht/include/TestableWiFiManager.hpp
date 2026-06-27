#ifndef TESTABLE_WIFI_MANAGER_HPP
#define TESTABLE_WIFI_MANAGER_HPP

#include "Handle.hpp"
#include "MockEventGroup.hpp"
#include "MockEventHandler.hpp"
#include "MockProvisioning.hpp"
#include "MockWiFi.hpp"
#include "WiFiManager.hpp"

class MockHandlerFactory {
  public:
    MOCK_METHOD(int, call, (MockEventGroup &));
};

struct ProvisionedWiFiManagerMockTypes {
    using WiFi = Handle<MockWiFi>;
    using EventGroup = MockEventGroup;
    using EventHandler = int;
    using Provisioning = MockProvisioningIsProvisioned;
};

struct NotProvisionedWiFiManagerMockTypes {
    using WiFi = Handle<MockWiFi>;
    using EventGroup = MockEventGroup;
    using EventHandler = int;
    using Provisioning = MockProvisioningNotProvisioned;
};

static_assert(WiFiManagerTypes<NotProvisionedWiFiManagerMockTypes>);
static_assert(WiFiManagerTypes<ProvisionedWiFiManagerMockTypes>);

class TestableProvisionedWiFiManager
    : public WiFiManager<ProvisionedWiFiManagerMockTypes> {
  public:
    using WiFiManager<ProvisionedWiFiManagerMockTypes>::WiFiManager;

    EGroup &get_event_group() { return event_group; };
    MockWiFi &get_wifi() { return Handle<MockWiFi>::get(); };
    std::optional<Provisioning> &get_provisioning() { return provisioning; };
};

class TestableNotProvisionedWiFiManager
    : public WiFiManager<NotProvisionedWiFiManagerMockTypes> {
  public:
    using WiFiManager<NotProvisionedWiFiManagerMockTypes>::WiFiManager;

    EGroup &get_event_group() { return event_group; };
    MockWiFi &get_wifi() { return Handle<MockWiFi>::get(); };
    std::optional<Provisioning> &get_provisioning() { return provisioning; };
};

#endif
