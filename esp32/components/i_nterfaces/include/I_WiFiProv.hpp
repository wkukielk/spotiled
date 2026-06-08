#ifndef I_WIFI_PROV_HPP
#define I_WIFI_PROV_HPP

#include "esp_err.h"
#include <concepts>
#include <cstdint>

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

template <typename T>
concept I_WiFiProv =
    std::default_initializable<T> &&
    requires(T t, const T ct, bool *provisioned, WiFiProvSecurity security,
             const void *wifi_prov_sec_params, const char *service_name,
             const char *service_key, uint8_t *uuid128) {
        { ct.is_provisioned(provisioned) } -> std::same_as<esp_err_t>;
        {
            t.start_provisioning(security, wifi_prov_sec_params, service_name,
                                 service_key)
        } -> std::same_as<esp_err_t>;
        { t.stop_provisioning() } -> std::same_as<void>;
        { t.reset_sm_state_on_failure() } -> std::same_as<esp_err_t>;
        { t.reset_provisioning() } -> std::same_as<esp_err_t>;
        { t.set_service_uuid(uuid128) } -> std::same_as<esp_err_t>;
    };

#endif
