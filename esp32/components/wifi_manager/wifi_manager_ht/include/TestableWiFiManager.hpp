#ifndef TESTABLE_WIFI_MANAGER_HPP
#define TESTABLE_WIFI_MANAGER_HPP

#include "Handle.hpp"
#include "MockEventGroup.hpp"
#include "MockEventHandler.hpp"
#include "MockProvisioning.hpp"
#include "MockWiFi.hpp"
#include "WiFiManager.hpp"

// Stands in for WiFiEventHandler; records the registered fail callback.
struct StubWiFiEventHandler {
    static void set_on_fail(std::function<void()> cb) {
        on_fail = std::move(cb);
    }
    inline static std::function<void()> on_fail{};
};

class MockHandlerFactory {
  public:
    MOCK_METHOD(StubWiFiEventHandler, call, (MockEventGroup &));
};

struct ProvisionedWiFiManagerMockTypes {
    using WiFi = Handle<::testing::NiceMock<MockWiFi>>;
    using EventGroup = MockEventGroup;
    using EventHandler = StubWiFiEventHandler;
    using Provisioning = MockProvisioningIsProvisioned;
};

struct NotProvisionedWiFiManagerMockTypes {
    using WiFi = Handle<::testing::NiceMock<MockWiFi>>;
    using EventGroup = MockEventGroup;
    using EventHandler = StubWiFiEventHandler;
    using Provisioning = MockProvisioningNotProvisioned;
};

static_assert(WiFiManagerTypes<NotProvisionedWiFiManagerMockTypes>);
static_assert(WiFiManagerTypes<ProvisionedWiFiManagerMockTypes>);

class TestableProvisionedWiFiManager
    : public WiFiManager<ProvisionedWiFiManagerMockTypes> {
  public:
    using WiFiManager<ProvisionedWiFiManagerMockTypes>::WiFiManager;
    using NiceWiFi = ::testing::NiceMock<MockWiFi>;

    NiceWiFi &get_wifi() { return Handle<NiceWiFi>::get(); }
    EGroup &get_event_group() { return event_group; };
    std::optional<Provisioning> &get_provisioning() { return provisioning; };
};

class TestableNotProvisionedWiFiManager
    : public WiFiManager<NotProvisionedWiFiManagerMockTypes> {
  public:
    using WiFiManager<NotProvisionedWiFiManagerMockTypes>::WiFiManager;
    using NiceWiFi = ::testing::NiceMock<MockWiFi>;

    EGroup &get_event_group() { return event_group; };
    NiceWiFi &get_wifi() { return Handle<NiceWiFi>::get(); };
    std::optional<Provisioning> &get_provisioning() { return provisioning; };
};

#endif
