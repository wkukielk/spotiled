#ifndef WIFI_WRAP_HPP
#define WIFI_WRAP_HPP

#include "I_WiFi.hpp"
#include "esp_wifi.h"

namespace wifi {
extern wifi_init_config_t cfg;
};

class WiFi : public I_WiFi {
  public:
    WiFi(const wifi_init_config_t *config);
    ~WiFi() noexcept override;
    esp_err_t start() override;
    esp_err_t stop() override;
    esp_err_t set_config(WiFiIF ifx, wifi_config_t *conf) override;
    esp_err_t set_mode(WiFiMode mode) override;
    esp_err_t set_ps(WiFiPSType pstype) override;
    esp_err_t connect() override;
    esp_err_t get_mac(WiFiIF ifx, uint8_t mac[6]) override;

    esp_netif_t *get_netif();

  private:
    esp_netif_t *netif;
};

#endif
