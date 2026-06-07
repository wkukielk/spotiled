#ifndef MOCK_WIFI
#define MOCK_WIFI

#include "I_WiFi.hpp"
#include <gmock/gmock.h>

class MockWiFi {
  public:
    MOCK_METHOD(esp_err_t, start, (), (noexcept));
    MOCK_METHOD(esp_err_t, stop, (), (noexcept));
    MOCK_METHOD(esp_err_t, set_config, (WiFiIF ifx, wifi_config_t *conf),
                (noexcept));
    MOCK_METHOD(esp_err_t, set_mode, (WiFiMode mode), (noexcept));
    MOCK_METHOD(esp_err_t, set_ps, (WiFiPSType pstype), (noexcept));
    MOCK_METHOD(esp_err_t, connect, (), (noexcept));
    MOCK_METHOD(esp_err_t, get_mac, (WiFiIF ifx, uint8_t mac[6]), (noexcept));

    static MockWiFi &get() { return *instance; };

    static void create() { instance = std::make_unique<MockWiFi>(); };
    static void cleanup() { instance = nullptr; };
    static std::unique_ptr<MockWiFi> instance;
};

static_assert(I_WiFi<MockWiFi>);

#endif
