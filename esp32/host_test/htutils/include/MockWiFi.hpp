#ifndef MOCK_WIFI
#define MOCK_WIFI

#include "I_WiFi.hpp"
#include <gmock/gmock.h>

class MockWiFi : public I_WiFi {
  public:
    MOCK_METHOD(esp_err_t, start, (), (override));
    MOCK_METHOD(esp_err_t, stop, (), (override));
    MOCK_METHOD(esp_err_t, set_mode, (WiFiMode mode), (override));
    MOCK_METHOD(esp_err_t, connect, (), (override));
    MOCK_METHOD(esp_err_t, get_mac, (WiFiIF ifx, uint8_t mac[6]), (override));
};

#endif
