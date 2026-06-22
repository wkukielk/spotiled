#include "BaseTest.hpp"
#include "HandlerAdapter.hpp"
#include "I_WiFiProv.hpp"
#include "MockDftEventLoop.hpp"
#include "MockWiFi.hpp"
#include "ProvEventHandler.hpp"
#include "esp_event.h"
#include "esp_wifi.h"
#include "wifi_aux.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using util::EventGroup;
using namespace std::ranges;

class ProvEventHandlerTest : public BaseTest {
  protected:
    DftEventLoopConfigurer dft_eloop{};
    MockDftEventLoop &eloop{MockDftEventLoop::reset_and_get()};
    EventGroup event_group{};
    ProvEventHandler<EventGroup, MockDftEventLoop> handler{event_group, 2};

    wifi_event_sta_disconnected_t disc_unrec{.reason = WIFI_REASON_AUTH_FAIL};
    wifi_event_sta_disconnected_t disc_rec{.reason =
                                               WIFI_REASON_CONNECTION_FAIL};
    ProvReason reason = ProvReason::AP_NOT_FOUND;

  public:
    void expect_state(const std::map<EventBits_t, bool> bits) {
        EXPECT_TRUE(all_of(bits, [this](auto bit) {
            return (bool)(event_group.get() & bit.first) == bit.second;
        }));
    }
};

TEST_F(ProvEventHandlerTest, init) {
    expect_state({{wifi::end_bit, true},
                  {wifi::fail_bit, false},
                  {wifi::success_bit, false}});
}

TEST_F(ProvEventHandlerTest, start) {
    eloop.post(NETWORK_PROV_EVENT, (int32_t)ProvEvent::START, nullptr, 0);
    expect_state({{wifi::end_bit, false},
                  {wifi::fail_bit, false},
                  {wifi::success_bit, false}});
}

TEST_F(ProvEventHandlerTest, success) {
    eloop.post(NETWORK_PROV_EVENT, (int32_t)ProvEvent::START, nullptr, 0);
    eloop.post(NETWORK_PROV_EVENT, (int32_t)ProvEvent::CRED_SUCCESS, nullptr,
               0);
    eloop.post(NETWORK_PROV_EVENT, (int32_t)ProvEvent::END, nullptr, 0);
    expect_state({{wifi::end_bit, true},
                  {wifi::fail_bit, false},
                  {wifi::success_bit, true}});
}

TEST_F(ProvEventHandlerTest, failThenEnd) {
    eloop.post(NETWORK_PROV_EVENT, (int32_t)ProvEvent::START, nullptr, 0);
    eloop.post(NETWORK_PROV_EVENT, (int32_t)ProvEvent::CRED_FAIL, &reason,
               sizeof(reason));
    eloop.post(NETWORK_PROV_EVENT, (int32_t)ProvEvent::END, nullptr, 0);
    expect_state({{wifi::end_bit, true},
                  {wifi::fail_bit, true},
                  {wifi::success_bit, false}});
}

TEST_F(ProvEventHandlerTest, failAndRecover) {
    eloop.post(NETWORK_PROV_EVENT, (int32_t)ProvEvent::START, nullptr, 0);
    eloop.post(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disc_rec,
               sizeof(disc_rec));
    eloop.post(NETWORK_PROV_EVENT, (int32_t)ProvEvent::CRED_SUCCESS, nullptr,
               0);
    expect_state({{wifi::end_bit, false},
                  {wifi::fail_bit, false},
                  {wifi::success_bit, true}});
}

TEST_F(ProvEventHandlerTest, tooManyRetries) {
    eloop.post(NETWORK_PROV_EVENT, (int32_t)ProvEvent::START, nullptr, 0);
    eloop.post(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disc_rec,
               sizeof(disc_rec));
    eloop.post(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disc_rec,
               sizeof(disc_rec));
    eloop.post(NETWORK_PROV_EVENT, (int32_t)ProvEvent::CRED_FAIL, &reason,
               sizeof(reason));
    expect_state({{wifi::end_bit, false},
                  {wifi::fail_bit, true},
                  {wifi::success_bit, false}});
}

TEST_F(ProvEventHandlerTest, reset) {
    eloop.post(NETWORK_PROV_EVENT, (int32_t)ProvEvent::START, nullptr, 0);
    eloop.post(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disc_rec,
               sizeof(disc_rec));
    eloop.post(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disc_rec,
               sizeof(disc_rec));
    eloop.post(NETWORK_PROV_EVENT, (int32_t)ProvEvent::END, nullptr, 0);
    eloop.post(NETWORK_PROV_EVENT, (int32_t)ProvEvent::START, nullptr, 0);
    expect_state({{wifi::end_bit, false},
                  {wifi::fail_bit, false},
                  {wifi::success_bit, false}});
}

TEST_F(ProvEventHandlerTest, retriesReset) {
    eloop.post(NETWORK_PROV_EVENT, (int32_t)ProvEvent::START, nullptr, 0);
    eloop.post(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disc_rec,
               sizeof(disc_rec));
    eloop.post(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disc_rec,
               sizeof(disc_rec));
    eloop.post(NETWORK_PROV_EVENT, (int32_t)ProvEvent::END, nullptr, 0);
    eloop.post(NETWORK_PROV_EVENT, (int32_t)ProvEvent::START, nullptr, 0);
    eloop.post(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disc_rec,
               sizeof(disc_rec));
    eloop.post(NETWORK_PROV_EVENT, (int32_t)ProvEvent::CRED_SUCCESS, nullptr,
               0);
    expect_state({{wifi::end_bit, false},
                  {wifi::fail_bit, false},
                  {wifi::success_bit, true}});
}