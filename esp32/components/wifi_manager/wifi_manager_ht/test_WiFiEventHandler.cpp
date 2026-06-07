#include "BaseTest.hpp"
#include "I_WiFi.hpp"
#include "Initializer.hpp"
#include "MockDftEventLoop.hpp"
#include "MockWiFi.hpp"
#include "WiFiEventHandler.hpp"
#include "esp_event.h"
#include "esp_wifi.h"
#include "wifi_aux.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace testing;

struct MockWiFiEventHandlerTypes {
    using EventGroup = util::EventGroup;
    using EventLoop = MockDftEventLoop;
    using WiFi = MockWiFi;
};

class WiFiEventHandlerTest : public Test {
  protected:
    DftEventLoopConfigurer dft_eloop{};
    MockDftEventLoop &eloop{MockDftEventLoop::reset_and_get()};
    util::EventGroup event_group{};
    Initializer<MockWiFi> wifi_{};
    WiFiEventHandler<MockWiFiEventHandlerTypes> handler{event_group, 2};

    wifi_event_sta_disconnected_t disc_unrec{.reason = WIFI_REASON_AUTH_FAIL};
    wifi_event_sta_disconnected_t disc_rec{.reason =
                                               WIFI_REASON_CONNECTION_FAIL};

  public:
    void expect_state(const std::map<EventBits_t, bool> bits) {
        for (auto bit : bits)
            EXPECT_TRUE((bool)(event_group.get() & bit.first) == bit.second);
    }
};

TEST_F(WiFiEventHandlerTest, init) {
    expect_state({{wifi::end_bit, true},
                  {wifi::fail_bit, false},
                  {wifi::success_bit, false}});
}

TEST_F(WiFiEventHandlerTest, start) {
    eloop.post(WIFI_EVENT, WIFI_EVENT_STA_START, nullptr, 0);
    expect_state({{wifi::end_bit, false},
                  {wifi::fail_bit, false},
                  {wifi::success_bit, false}});
}

TEST_F(WiFiEventHandlerTest, success) {
    eloop.post(WIFI_EVENT, WIFI_EVENT_STA_START, nullptr, 0);
    eloop.post(IP_EVENT, IP_EVENT_STA_GOT_IP, nullptr, 0);
    expect_state({{wifi::end_bit, false},
                  {wifi::fail_bit, false},
                  {wifi::success_bit, true}});
}

TEST_F(WiFiEventHandlerTest, fail) {
    eloop.post(WIFI_EVENT, WIFI_EVENT_STA_START, nullptr, 0);
    eloop.post(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disc_unrec,
               sizeof(disc_unrec));
    expect_state({{wifi::end_bit, false},
                  {wifi::fail_bit, true},
                  {wifi::success_bit, false}});
}

TEST_F(WiFiEventHandlerTest, noFailIfRecoverable) {
    eloop.post(WIFI_EVENT, WIFI_EVENT_STA_START, nullptr, 0);
    eloop.post(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disc_rec,
               sizeof(disc_rec));
    expect_state({{wifi::end_bit, false},
                  {wifi::fail_bit, false},
                  {wifi::success_bit, false}});
}

TEST_F(WiFiEventHandlerTest, tooManyRetries) {
    eloop.post(WIFI_EVENT, WIFI_EVENT_STA_START, nullptr, 0);
    eloop.post(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disc_rec,
               sizeof(disc_rec));
    eloop.post(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disc_rec,
               sizeof(disc_rec));
    expect_state({{wifi::end_bit, false},
                  {wifi::fail_bit, true},
                  {wifi::success_bit, false}});
}

TEST_F(WiFiEventHandlerTest, recover) {
    eloop.post(WIFI_EVENT, WIFI_EVENT_STA_START, nullptr, 0);
    eloop.post(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disc_rec,
               sizeof(disc_rec));
    eloop.post(IP_EVENT, IP_EVENT_STA_GOT_IP, nullptr, 0);
    expect_state({{wifi::end_bit, false},
                  {wifi::fail_bit, false},
                  {wifi::success_bit, true}});
}

TEST_F(WiFiEventHandlerTest, connectThenDisconnect) {
    eloop.post(WIFI_EVENT, WIFI_EVENT_STA_START, nullptr, 0);
    eloop.post(IP_EVENT, IP_EVENT_STA_GOT_IP, nullptr, 0);
    eloop.post(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disc_rec,
               sizeof(disc_rec));
    expect_state({{wifi::end_bit, false},
                  {wifi::fail_bit, true},
                  {wifi::success_bit, false}});
}

TEST_F(WiFiEventHandlerTest, connectThenStop) {
    eloop.post(WIFI_EVENT, WIFI_EVENT_STA_START, nullptr, 0);
    eloop.post(IP_EVENT, IP_EVENT_STA_GOT_IP, nullptr, 0);
    eloop.post(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disc_rec,
               sizeof(disc_rec));
    eloop.post(WIFI_EVENT, WIFI_EVENT_STA_STOP, nullptr, 0);
    expect_state({{wifi::end_bit, true},
                  {wifi::fail_bit, false},
                  {wifi::success_bit, false}});
}

TEST_F(WiFiEventHandlerTest, failThenConnect) {
    eloop.post(WIFI_EVENT, WIFI_EVENT_STA_START, nullptr, 0);
    eloop.post(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disc_unrec,
               sizeof(disc_rec));
    eloop.post(IP_EVENT, IP_EVENT_STA_GOT_IP, nullptr, 0);
    expect_state({{wifi::end_bit, false},
                  {wifi::fail_bit, false},
                  {wifi::success_bit, true}});
}
