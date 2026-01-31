#ifndef I_WIFI_PROV_HPP
#define I_WIFI_PROV_HPP

#include "esp_err.h"

enum class ProvEvent {
    INIT,
    START,
    SET_STA_CONFIG,
    CRED_RECV,
    CRED_FAIL,
    CRED_SUCCESS,
    END,
    DEINIT,
};

enum class ProvReason { AUTH_ERROR, AP_NOT_FOUND, OK };

enum class WiFiProvSecurity { S0 = 0, S1 = 1, S2 = 2 };

class I_WiFiProv {
  public:
    virtual ~I_WiFiProv() noexcept {};
    virtual esp_err_t is_provisioned(bool *provisioned) = 0;
    virtual esp_err_t start_provisioning(WiFiProvSecurity security,
                                         const void *wifi_prov_sec_params,
                                         const char *service_name,
                                         const char *service_key) = 0;
    virtual void stop_provisioning() = 0;
    virtual esp_err_t reset_sm_state_on_failure() = 0;
    virtual esp_err_t reset_provisioning() = 0;
    virtual esp_err_t set_service_uuid(uint8_t *uuid128) = 0;
};

#endif