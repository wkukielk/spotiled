#ifndef MOCK_WIFI_MANAGER_HPP
#define MOCK_WIFI_MANAGER_HPP

#include "WiFiManager.hpp"
#include <gmock/gmock.h>

class MockWiFiManager : public I_WiFiManager {
  public:
    MOCK_METHOD(void, init, (), (override));
    MOCK_METHOD(int, connect, (), (override));
    MOCK_METHOD(int, disconnect, (), (override));
    MOCK_METHOD(bool, is_connected, (), (override));
};

#endif
