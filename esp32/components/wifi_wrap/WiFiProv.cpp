#include "WiFiProv.hpp"
#include "esp_log.h"
#include "util.hpp"

wifi_prov_mgr_config_t wifi_prov::cfg = {
    .scheme = wifi_prov_scheme_ble,
    .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM};

WiFiProv::WiFiProv(wifi_prov_mgr_config_t *config) {
    UTIL_CHECK(wifi_prov_mgr_init(*config), "Failed to init prov");
    ESP_LOGI("WiFiProv", "WiFiProv init");
}

WiFiProv::~WiFiProv() noexcept {
    wifi_prov_mgr_deinit();
    ESP_LOGI("WiFiProv", "WiFiProv deinit");
}
esp_err_t WiFiProv::is_provisioned(bool *provisioned) {
    return wifi_prov_mgr_is_provisioned(provisioned);
}
esp_err_t WiFiProv::start_provisioning(WiFiProvSecurity security,
                                       const void *wifi_prov_sec_params,
                                       const char *service_name,
                                       const char *service_key) {
    return wifi_prov_mgr_start_provisioning((wifi_prov_security_t)security,
                                            wifi_prov_sec_params, service_name,
                                            service_key);
}
void WiFiProv::stop_provisioning() { wifi_prov_mgr_stop_provisioning(); }
esp_err_t WiFiProv::reset_sm_state_on_failure() {
    return wifi_prov_mgr_reset_sm_state_on_failure();
}
esp_err_t WiFiProv::reset_provisioning() {
    return wifi_prov_mgr_reset_provisioning();
}

esp_err_t WiFiProv::set_service_uuid(uint8_t *uuid128) {
    return wifi_prov_scheme_ble_set_service_uuid(uuid128);
}