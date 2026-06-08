#ifndef MOCK_WIFI_PROV
#define MOCK_WIFI_PROV

#include "I_WiFiProv.hpp"
#include <gmock/gmock.h>

class MockWiFiProv {
  public:
    MOCK_METHOD(esp_err_t, is_provisioned, (bool *provisioned), (const));
    MOCK_METHOD(esp_err_t, start_provisioning,
                (WiFiProvSecurity security, const void *wifi_prov_sec_params,
                 const char *service_name, const char *service_key));
    MOCK_METHOD(void, stop_provisioning, ());
    MOCK_METHOD(esp_err_t, reset_sm_state_on_failure, ());
    MOCK_METHOD(esp_err_t, reset_provisioning, ());
    MOCK_METHOD(esp_err_t, set_service_uuid, (uint8_t *uuid128));
};

static_assert(I_WiFiProv<MockWiFiProv>);

#endif
