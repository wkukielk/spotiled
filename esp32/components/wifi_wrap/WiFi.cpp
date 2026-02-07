#include "WiFi.hpp"
#include "errors.hpp"
#include "esp_log.h"
#include "esp_wifi.h"
#include "util.hpp"

wifi_init_config_t wifi::cfg = WIFI_INIT_CONFIG_DEFAULT();

WiFi::WiFi(const wifi_init_config_t *config) {
    netif = esp_netif_create_default_wifi_sta();
    UTIL_THROW(netif == nullptr, InitError, "create wifi sta");
    UTIL_THROW(esp_wifi_init(config), InitError, "wifi init");
    ESP_LOGI("WIFI", "WiFi init");
}
WiFi::~WiFi() noexcept {
    esp_wifi_deinit();
    esp_netif_destroy_default_wifi(netif);
    ESP_LOGI("WIFI", "WiFi deinit");
}
esp_err_t WiFi::start() { return esp_wifi_start(); }
esp_err_t WiFi::stop() { return esp_wifi_stop(); }
esp_err_t WiFi::set_config(WiFiIF ifx, wifi_config_t *conf) {
    return esp_wifi_set_config((wifi_interface_t)ifx, conf);
}
esp_err_t WiFi::set_mode(WiFiMode mode) {
    return esp_wifi_set_mode((wifi_mode_t)mode);
}
esp_err_t WiFi::set_ps(WiFiPSType pstype) {
    return esp_wifi_set_ps((wifi_ps_type_t)pstype);
}
esp_err_t WiFi::connect() { return esp_wifi_connect(); }
esp_err_t WiFi::get_mac(WiFiIF ifx, uint8_t mac[6]) {
    return esp_wifi_get_mac((wifi_interface_t)ifx, mac);
}

esp_netif_t *WiFi::get_netif() { return netif; }
