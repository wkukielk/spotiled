#include "BaseTest.hpp"
#include "HandlerAdapter.hpp"
#include "I_WiFiProv.hpp"
#include "MockWiFi.hpp"
#include "ProvEventHandler.hpp"
#include "esp_event.h"
#include "esp_wifi.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

class ProvEventHandlerTest : public BaseTest {
  protected:
    ProvEventHandlerTest() {
        handler = std::make_unique<ProvEventHandler>(event_group, 2,
                                                     util::Register::NO);
        aux = std::make_unique<HandlerAdapter>(*handler);
    }

    util::EventGroup event_group{};
    std::unique_ptr<ProvEventHandler> handler;
    std::unique_ptr<HandlerAdapter> aux;

    wifi_event_sta_disconnected_t disc_unrec{.reason = WIFI_REASON_AUTH_FAIL};
    wifi_event_sta_disconnected_t disc_rec{.reason =
                                               WIFI_REASON_CONNECTION_FAIL};
    ProvReason reason = ProvReason::AP_NOT_FOUND;

  public:
    void expect_state(const std::map<EventBits_t, bool> bits) {
        for (auto bit : bits)
            EXPECT_TRUE((bool)(event_group.get() & bit.first) == bit.second);
    }
};

TEST_F(ProvEventHandlerTest, init) {
    expect_state({{util::end_bit, true},
                  {util::fail_bit, false},
                  {util::success_bit, false}});
}

TEST_F(ProvEventHandlerTest, start) {
    aux->post(nullptr, 0, WIFI_PROV_EVENT, (int32_t)ProvEvent::START);
    expect_state({{util::end_bit, false},
                  {util::fail_bit, false},
                  {util::success_bit, false}});
}

TEST_F(ProvEventHandlerTest, success) {
    aux->post(nullptr, 0, WIFI_PROV_EVENT, (int32_t)ProvEvent::START);
    aux->post(nullptr, 0, WIFI_PROV_EVENT, (int32_t)ProvEvent::CRED_SUCCESS);
    aux->post(nullptr, 0, WIFI_PROV_EVENT, (int32_t)ProvEvent::END);
    expect_state({{util::end_bit, true},
                  {util::fail_bit, false},
                  {util::success_bit, true}});
}

TEST_F(ProvEventHandlerTest, failThenEnd) {
    aux->post(nullptr, 0, WIFI_PROV_EVENT, (int32_t)ProvEvent::START);
    aux->post(&reason, sizeof(reason), WIFI_PROV_EVENT,
              (int32_t)ProvEvent::CRED_FAIL);
    aux->post(nullptr, 0, WIFI_PROV_EVENT, (int32_t)ProvEvent::END);
    expect_state({{util::end_bit, true},
                  {util::fail_bit, true},
                  {util::success_bit, false}});
}

TEST_F(ProvEventHandlerTest, failAndRecover) {
    aux->post(nullptr, 0, WIFI_PROV_EVENT, (int32_t)ProvEvent::START);
    aux->post(&disc_rec, sizeof(disc_rec), WIFI_EVENT,
              WIFI_EVENT_STA_DISCONNECTED);
    aux->post(nullptr, 0, WIFI_PROV_EVENT, (int32_t)ProvEvent::CRED_SUCCESS);
    expect_state({{util::end_bit, false},
                  {util::fail_bit, false},
                  {util::success_bit, true}});
}

TEST_F(ProvEventHandlerTest, tooManyRetries) {
    aux->post(nullptr, 0, WIFI_PROV_EVENT, (int32_t)ProvEvent::START);
    aux->post(&disc_rec, sizeof(disc_rec), WIFI_EVENT,
              WIFI_EVENT_STA_DISCONNECTED);
    aux->post(&disc_rec, sizeof(disc_rec), WIFI_EVENT,
              WIFI_EVENT_STA_DISCONNECTED);
    aux->post(&reason, sizeof(reason), WIFI_PROV_EVENT,
              (int32_t)ProvEvent::CRED_FAIL);
    expect_state({{util::end_bit, false},
                  {util::fail_bit, true},
                  {util::success_bit, false}});
}

TEST_F(ProvEventHandlerTest, reset) {
    aux->post(nullptr, 0, WIFI_PROV_EVENT, (int32_t)ProvEvent::START);
    aux->post(&disc_rec, sizeof(disc_rec), WIFI_EVENT,
              WIFI_EVENT_STA_DISCONNECTED);
    aux->post(&disc_rec, sizeof(disc_rec), WIFI_EVENT,
              WIFI_EVENT_STA_DISCONNECTED);
    aux->post(nullptr, 0, WIFI_PROV_EVENT, (int32_t)ProvEvent::END);
    aux->post(nullptr, 0, WIFI_PROV_EVENT, (int32_t)ProvEvent::START);
    expect_state({{util::end_bit, false},
                  {util::fail_bit, false},
                  {util::success_bit, false}});
}

TEST_F(ProvEventHandlerTest, retriesReset) {
    aux->post(nullptr, 0, WIFI_PROV_EVENT, (int32_t)ProvEvent::START);
    aux->post(&disc_rec, sizeof(disc_rec), WIFI_EVENT,
              WIFI_EVENT_STA_DISCONNECTED);
    aux->post(&disc_rec, sizeof(disc_rec), WIFI_EVENT,
              WIFI_EVENT_STA_DISCONNECTED);
    aux->post(nullptr, 0, WIFI_PROV_EVENT, (int32_t)ProvEvent::END);
    aux->post(nullptr, 0, WIFI_PROV_EVENT, (int32_t)ProvEvent::START);
    aux->post(&disc_rec, sizeof(disc_rec), WIFI_EVENT,
              WIFI_EVENT_STA_DISCONNECTED);
    aux->post(nullptr, 0, WIFI_PROV_EVENT, (int32_t)ProvEvent::CRED_SUCCESS);
    expect_state({{util::end_bit, false},
                  {util::fail_bit, false},
                  {util::success_bit, true}});
}