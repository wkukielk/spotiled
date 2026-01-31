#ifndef MOCK_PROVISIONING
#define MOCK_PROVISIONING

#include "Provisioning.hpp"
#include <gmock/gmock.h>

class MockProvisioning : public I_Provisioning {
  public:
    MOCK_METHOD(bool, is_provisioned, (), (override));
    MOCK_METHOD(int, connect, (), (override));
    MOCK_METHOD(I_WiFiProv &, get_wifi_prov, (), (override));
    MOCK_METHOD(esp_err_t, on, (), (override));
    MOCK_METHOD(esp_err_t, off, (), (override));
    MOCK_METHOD(bool, is_on, (), (const, override));
};

#endif
