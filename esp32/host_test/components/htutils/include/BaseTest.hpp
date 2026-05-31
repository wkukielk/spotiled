#ifndef BASE_TEST_HPP
#define BASE_TEST_HPP

#include "MockMDNS.hpp"
#include "MockTime.hpp"
#include "MockWiFi.hpp"
#include "global.hpp"
#include <gtest/gtest.h>

class BaseTest : public testing::Test {
  protected:
    BaseTest() {
        auto &global = Global::get_global();
        global.set_wifi(std::make_unique<MockWiFi>());
        global.set_time(std::make_unique<MockTime>());
        global.set_mdns(std::make_unique<MockMDNS>());
        wifi_m = static_cast<MockWiFi *>(&wifi_());
        time_m = static_cast<MockTime *>(&time());
        mdns_m = static_cast<MockMDNS *>(&mdns_());
    }

    ~BaseTest() noexcept {
        auto &global = Global::get_global();
        global.set_mdns(nullptr);
        global.set_time(nullptr);
        global.set_wifi(nullptr);
    }

    MockWiFi *wifi_m{};
    MockTime *time_m{};
    MockMDNS *mdns_m{};
};

#endif
