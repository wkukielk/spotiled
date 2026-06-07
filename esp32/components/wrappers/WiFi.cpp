#include "WiFi.hpp"
#include "errors.hpp"
#include "esp_log.h"
#include "esp_wifi.h"
#include "util.hpp"
#include <utility>

wifi_init_config_t wifi::cfg = WIFI_INIT_CONFIG_DEFAULT();

std::optional<WiFi> WiFi::instance;

WiFi::WiFi(const wifi_init_config_t *config) {
    netif = esp_netif_create_default_wifi_sta();
    UTIL_THROW(netif == nullptr, InitError, "create wifi sta");
    try {
        UTIL_THROW(esp_wifi_init(config), InitError, "wifi init");
    } catch (...) {
        esp_netif_destroy_default_wifi(netif);
        throw;
    }
    ESP_LOGI("WIFI", "WiFi init");
}

WiFi::~WiFi() noexcept {
    if (auto err = esp_wifi_deinit(); err != ESP_OK)
        ESP_LOGW("WIFI", "deinit failed: %d", err);
    esp_netif_destroy_default_wifi(netif);
    ESP_LOGI("WIFI", "WiFi deinit");
}

esp_err_t WiFi::create(const wifi_init_config_t *config) {
    instance.emplace(config);
    return ESP_OK;
}

esp_err_t WiFi::cleanup() noexcept {
    instance.reset();
    return ESP_OK;
}

esp_err_t WiFi::start() noexcept { return esp_wifi_start(); }
esp_err_t WiFi::stop() noexcept { return esp_wifi_stop(); }
esp_err_t WiFi::set_config(WiFiIF ifx, wifi_config_t *conf) noexcept {
    return esp_wifi_set_config((wifi_interface_t)ifx, conf);
}
esp_err_t WiFi::set_mode(WiFiMode mode) noexcept {
    return esp_wifi_set_mode((wifi_mode_t)mode);
}
esp_err_t WiFi::set_ps(WiFiPSType pstype) noexcept {
    return esp_wifi_set_ps((wifi_ps_type_t)pstype);
}
esp_err_t WiFi::connect() noexcept { return esp_wifi_connect(); }
esp_err_t WiFi::get_mac(WiFiIF ifx, uint8_t mac[6]) noexcept {
    return esp_wifi_get_mac((wifi_interface_t)ifx, mac);
}

esp_netif_t *WiFi::get_netif() noexcept { return netif; }

WiFiInitializer::WiFiInitializer() { WiFi::create(&wifi::cfg); }

WiFiInitializer::~WiFiInitializer() { WiFi::cleanup(); }
