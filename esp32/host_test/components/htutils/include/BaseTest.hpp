#ifndef BASE_TEST_HPP
#define BASE_TEST_HPP

#include "MockTime.hpp"
#include "MockWiFi.hpp"
#include "global.hpp"
#include <gtest/gtest.h>

class BaseTest : public testing::Test {
  protected:
    BaseTest() {
        auto *global = Global::get_global();
        global->set_wifi(std::make_unique<MockWiFi>());
        global->set_time(std::make_unique<MockTime>());
        wifi = static_cast<MockWiFi *>(&wifi());
        time = static_cast<MockTime *>(&time());
    }

    ~BaseTest() noexcept {
        auto *global = Global::get_global();
        global->set_wifi(nullptr);
        global->set_time(nullptr);
    }

    MockWiFi *wifi{};
    MockTime *time{};
};

#endif
