#ifndef TESTABLE_PROVISIONING_HPP
#define TESTABLE_PROVISIONING_HPP

#include "MockEventGroup.hpp"
#include "MockEventHandler.hpp"
#include "MockWiFiProv.hpp"
#include "Provisioning.hpp"

class MockProvEventHandler {
  public:
    MockProvEventHandler(MockEventGroup &, uint8_t) {};
};

struct MockProvisioningTypes {
    using WiFiProv = ::testing::NiceMock<MockWiFiProv>;
    using EventGroup = MockEventGroup;
    using EventHandler = MockProvEventHandler;
};
static_assert(ProvisioningTypes<MockProvisioningTypes>);

class TestableProvisioning : public Provisioning<MockProvisioningTypes> {
  public:
    using Provisioning<MockProvisioningTypes>::Provisioning;
    MockWiFiProv &get_prov() { return wifi_prov; }
    MockProvEventHandler &get_handler() { return handler; }
};

#endif
