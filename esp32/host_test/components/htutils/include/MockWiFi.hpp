#ifndef MOCK_WIFI
#define MOCK_WIFI

#include "I_WiFi.hpp"
#include <gmock/gmock.h>

class MockWiFi {
  public:
    esp_err_t init() noexcept { return ESP_OK; }
    esp_err_t deinit() noexcept { return ESP_OK; }
    MOCK_METHOD(esp_err_t, start, (), (noexcept));
    MOCK_METHOD(esp_err_t, stop, (), (noexcept));
    MOCK_METHOD(esp_err_t, set_config, (WiFiIF ifx, wifi_config_t *conf),
                (noexcept));
    MOCK_METHOD(esp_err_t, set_mode, (WiFiMode mode), (noexcept));
    MOCK_METHOD(esp_err_t, set_ps, (WiFiPSType pstype), (noexcept));
    MOCK_METHOD(esp_err_t, connect, (), (noexcept));
    MOCK_METHOD(esp_err_t, get_mac, (WiFiIF ifx, uint8_t mac[6]), (noexcept));
};

static_assert(I_WiFi<MockWiFi>);

#endif
