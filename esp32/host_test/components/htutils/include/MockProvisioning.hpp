#ifndef MOCK_PROVISIONING
#define MOCK_PROVISIONING

#include "Provisioning.hpp"
#include <gmock/gmock.h>

class MockProvisioningIsProvisioned {
  public:
    using EventGroup = MockEventGroup;
    MockProvisioningIsProvisioned(MockEventGroup &, std::string_view) {};
    bool is_provisioned() const { return true; }
    MOCK_METHOD(int, connect, ());
    MOCK_METHOD(int, request_reprovision, ());
};

class MockProvisioningNotProvisioned {
  public:
    using EventGroup = MockEventGroup;
    MockProvisioningNotProvisioned(MockEventGroup &, std::string_view) {};
    bool is_provisioned() const { return false; }
    MOCK_METHOD(int, connect, ());
    MOCK_METHOD(int, request_reprovision, ());
};

static_assert(I_Provisioning<MockProvisioningIsProvisioned>);
static_assert(I_Provisioning<MockProvisioningNotProvisioned>);

#endif
