#include "WiFiProv.hpp"
#include "errors.hpp"
#include "esp_log.h"
#include "util.hpp"

network_prov_mgr_config_t wifi_prov::cfg = {
    .scheme = network_prov_scheme_ble,
    .scheme_event_handler = NETWORK_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM,
    .app_event_handler = NETWORK_PROV_EVENT_HANDLER_NONE,
    .network_prov_wifi_conn_cfg{}};

WiFiProv::WiFiProv() : WiFiProv{&wifi_prov::cfg} {};

WiFiProv::WiFiProv(network_prov_mgr_config_t *config) {
    UTIL_THROW(network_prov_mgr_init(*config), InitError, "prov");
    ESP_LOGI("WiFiProv", "WiFiProv init");
}

WiFiProv::~WiFiProv() noexcept {
    network_prov_mgr_deinit();
    ESP_LOGI("WiFiProv", "WiFiProv deinit");
}
esp_err_t WiFiProv::is_provisioned(bool *provisioned) const {
    return network_prov_mgr_is_wifi_provisioned(provisioned);
}
esp_err_t WiFiProv::start_provisioning(WiFiProvSecurity security,
                                       const void *network_prov_sec_params,
                                       const char *service_name,
                                       const char *service_key) {
    return network_prov_mgr_start_provisioning(
        (network_prov_security_t)security, network_prov_sec_params,
        service_name, service_key);
}
void WiFiProv::stop_provisioning() { network_prov_mgr_stop_provisioning(); }
esp_err_t WiFiProv::reset_sm_state_on_failure() {
    return network_prov_mgr_reset_wifi_sm_state_on_failure();
}
esp_err_t WiFiProv::reset_provisioning() {
    return network_prov_mgr_reset_wifi_provisioning();
}

esp_err_t WiFiProv::set_service_uuid(uint8_t *uuid128) {
    return network_prov_scheme_ble_set_service_uuid(uuid128);
}
