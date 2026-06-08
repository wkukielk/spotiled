#ifndef MOCK_PROVISIONING
#define MOCK_PROVISIONING

#include "Provisioning.hpp"
#include <gmock/gmock.h>

class MockProvisioning {
  public:
    MOCK_METHOD(bool, is_provisioned, (), (const));
    MOCK_METHOD(int, connect, ());
};

static_assert(I_Provisioning<MockProvisioning>);

#endif
