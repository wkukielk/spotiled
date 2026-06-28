#include "WiFi.hpp"
#include "errors.hpp"
#include "esp_log.h"
#include "esp_wifi.h"
#include "util.hpp"

wifi_init_config_t wifi::cfg = WIFI_INIT_CONFIG_DEFAULT();

esp_err_t WiFi::init(const wifi_init_config_t *config) noexcept {
    netif = esp_netif_create_default_wifi_sta();
    UTIL_RETURN(netif == nullptr);
    auto err = esp_wifi_init(config);
    if (err != ESP_OK)
        esp_netif_destroy_default_wifi(netif);
    return err;
}

esp_err_t WiFi::deinit() noexcept {
    auto err = esp_wifi_deinit();
    if (err != ESP_OK)
        ESP_LOGW("WIFI", "deinit failed: %d", err);
    esp_netif_destroy_default_wifi(netif);
    ESP_LOGI("WIFI", "WiFi deinit");
    return err;
}

