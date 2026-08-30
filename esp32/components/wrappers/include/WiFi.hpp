#ifndef WIFI_WRAP_HPP
#define WIFI_WRAP_HPP

#include "I_WiFi.hpp"
#include "esp_wifi.h"
#include <optional>

namespace wifi {
extern wifi_init_config_t cfg;
};

class WiFi {
  public:
    esp_err_t init(const wifi_init_config_t *config) noexcept;
    esp_err_t deinit() noexcept;

    esp_err_t init() noexcept { return init(&wifi::cfg); }
    esp_err_t start() noexcept { return esp_wifi_start(); }
    esp_err_t stop() noexcept { return esp_wifi_stop(); }
    esp_err_t disconnect() noexcept { return esp_wifi_disconnect(); }
    esp_err_t set_config(WiFiIF ifx, wifi_config_t *conf) noexcept {
        return esp_wifi_set_config((wifi_interface_t)ifx, conf);
    }
    esp_err_t set_mode(WiFiMode mode) noexcept {
        return esp_wifi_set_mode((wifi_mode_t)mode);
    }
    esp_err_t set_ps(WiFiPSType pstype) noexcept {
        return esp_wifi_set_ps((wifi_ps_type_t)pstype);
    }
    esp_err_t connect() noexcept { return esp_wifi_connect(); }
    esp_err_t get_mac(WiFiIF ifx, uint8_t mac[6]) noexcept {
        return esp_wifi_get_mac((wifi_interface_t)ifx, mac);
    }
    esp_netif_t *get_netif() noexcept { return netif; }

  private:
    esp_netif_t *netif;
};

static_assert(I_WiFi<WiFi>);

#endif
