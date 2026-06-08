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
    static WiFi &get() noexcept { return *instance; }
    static esp_err_t create(const wifi_init_config_t *config);
    static esp_err_t create();
    static esp_err_t cleanup() noexcept;

    WiFi(const wifi_init_config_t *config);
    ~WiFi() noexcept;

    esp_err_t start() noexcept;
    esp_err_t stop() noexcept;
    esp_err_t set_config(WiFiIF ifx, wifi_config_t *conf) noexcept;
    esp_err_t set_mode(WiFiMode mode) noexcept;
    esp_err_t set_ps(WiFiPSType pstype) noexcept;
    esp_err_t connect() noexcept;
    esp_err_t get_mac(WiFiIF ifx, uint8_t mac[6]) noexcept;

    esp_netif_t *get_netif() noexcept;

  private:
    esp_netif_t *netif;
    static std::optional<WiFi> instance;
};

static_assert(I_WiFi<WiFi>);

class WiFiInitializer {
  public:
    WiFiInitializer();
    ~WiFiInitializer();
};

#endif
