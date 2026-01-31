#include "BaseTest.hpp"
#include "HandlerAdapter.hpp"
#include "I_WiFi.hpp"
#include "MockWiFi.hpp"
#include "WiFiEventHandler.hpp"
#include "esp_event.h"
#include "esp_wifi.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

class WiFiEventHandlerTest : public BaseTest {
  protected:
    WiFiEventHandlerTest() {
        handler = std::make_unique<WiFiEventHandler>(event_group, 2,
                                                     util::Register::NO);
        aux = std::make_unique<HandlerAdapter>(*handler);
    }

    util::EventGroup event_group{};
    std::unique_ptr<WiFiEventHandler> handler;
    std::unique_ptr<HandlerAdapter> aux;

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
    expect_state({{util::end_bit, true},
                  {util::fail_bit, false},
                  {util::success_bit, false}});
}

TEST_F(WiFiEventHandlerTest, start) {
    aux->post(nullptr, 0, WIFI_EVENT, WIFI_EVENT_STA_START);
    expect_state({{util::end_bit, false},
                  {util::fail_bit, false},
                  {util::success_bit, false}});
}

TEST_F(WiFiEventHandlerTest, success) {
    aux->post(nullptr, 0, WIFI_EVENT, WIFI_EVENT_STA_START);
    aux->post(nullptr, 0, IP_EVENT, IP_EVENT_STA_GOT_IP);
    expect_state({{util::end_bit, false},
                  {util::fail_bit, false},
                  {util::success_bit, true}});
}

TEST_F(WiFiEventHandlerTest, fail) {
    aux->post(nullptr, 0, WIFI_EVENT, WIFI_EVENT_STA_START);
    aux->post(&disc_unrec, sizeof(disc_unrec), WIFI_EVENT,
              WIFI_EVENT_STA_DISCONNECTED);
    expect_state({{util::end_bit, false},
                  {util::fail_bit, true},
                  {util::success_bit, false}});
}

TEST_F(WiFiEventHandlerTest, noFailIfRecoverable) {
    aux->post(nullptr, 0, WIFI_EVENT, WIFI_EVENT_STA_START);
    aux->post(&disc_rec, sizeof(disc_rec), WIFI_EVENT,
              WIFI_EVENT_STA_DISCONNECTED);
    expect_state({{util::end_bit, false},
                  {util::fail_bit, false},
                  {util::success_bit, false}});
}

TEST_F(WiFiEventHandlerTest, tooManyRetries) {
    aux->post(nullptr, 0, WIFI_EVENT, WIFI_EVENT_STA_START);
    aux->post(&disc_rec, sizeof(disc_rec), WIFI_EVENT,
              WIFI_EVENT_STA_DISCONNECTED);
    aux->post(&disc_rec, sizeof(disc_rec), WIFI_EVENT,
              WIFI_EVENT_STA_DISCONNECTED);
    expect_state({{util::end_bit, false},
                  {util::fail_bit, true},
                  {util::success_bit, false}});
}

TEST_F(WiFiEventHandlerTest, recover) {
    aux->post(nullptr, 0, WIFI_EVENT, WIFI_EVENT_STA_START);
    aux->post(&disc_rec, sizeof(disc_rec), WIFI_EVENT,
              WIFI_EVENT_STA_DISCONNECTED);
    aux->post(nullptr, 0, IP_EVENT, IP_EVENT_STA_GOT_IP);
    expect_state({{util::end_bit, false},
                  {util::fail_bit, false},
                  {util::success_bit, true}});
}

TEST_F(WiFiEventHandlerTest, connectThenDisconnect) {
    aux->post(nullptr, 0, WIFI_EVENT, WIFI_EVENT_STA_START);
    aux->post(nullptr, 0, IP_EVENT, IP_EVENT_STA_GOT_IP);
    aux->post(&disc_rec, sizeof(disc_rec), WIFI_EVENT,
              WIFI_EVENT_STA_DISCONNECTED);
    expect_state({{util::end_bit, false},
                  {util::fail_bit, true},
                  {util::success_bit, false}});
}

TEST_F(WiFiEventHandlerTest, connectThenStop) {
    aux->post(nullptr, 0, WIFI_EVENT, WIFI_EVENT_STA_START);
    aux->post(nullptr, 0, IP_EVENT, IP_EVENT_STA_GOT_IP);
    aux->post(&disc_rec, sizeof(disc_rec), WIFI_EVENT,
              WIFI_EVENT_STA_DISCONNECTED);
    aux->post(nullptr, 0, WIFI_EVENT, WIFI_EVENT_STA_STOP);
    expect_state({{util::end_bit, true},
                  {util::fail_bit, false},
                  {util::success_bit, false}});
}

TEST_F(WiFiEventHandlerTest, failThenConnect) {
    aux->post(nullptr, 0, WIFI_EVENT, WIFI_EVENT_STA_START);
    aux->post(&disc_unrec, sizeof(disc_rec), WIFI_EVENT,
              WIFI_EVENT_STA_DISCONNECTED);
    aux->post(nullptr, 0, IP_EVENT, IP_EVENT_STA_GOT_IP);
    expect_state({{util::end_bit, false},
                  {util::fail_bit, false},
                  {util::success_bit, true}});
}
