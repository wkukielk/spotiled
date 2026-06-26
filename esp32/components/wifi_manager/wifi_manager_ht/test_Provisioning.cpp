#include "I_WiFi.hpp"
#include "MockTime.hpp"
#include "Provisioning.hpp"
#include "TestableProvisioning.hpp"
#include "esp_event.h"
#include "wifi_aux.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace testing;

class ProvisioningTest : public Test {
  protected:
    TimeHandle time_handle{mkuptr<MockTime>()};
    MockTime &time = (MockTime &)(Global::get_time());
    const char *service_name = "TEST_PROV";
    MockEventGroup event_group{};
    TestableProvisioning prov{event_group, service_name};
    MockWiFiProv &wifi_prov{prov.get_prov()};
    MockProvEventHandler &handler{prov.get_handler()};
};

using wifi::end_bit;
using wifi::fail_bit;
using wifi::success_bit;

TEST_F(ProvisioningTest, isProvisioned) {
    EXPECT_CALL(wifi_prov, is_provisioned(_)).WillOnce(Return(ESP_FAIL));
    EXPECT_THROW(prov.is_provisioned(), WiFiManagerError);
}

TEST_F(ProvisioningTest, success) {
    EXPECT_CALL(wifi_prov, start_provisioning(_, _, StrEq(service_name), _));
    EXPECT_CALL(event_group, wait(success_bit | fail_bit))
        .WillOnce(Return(success_bit));
    EXPECT_EQ(prov.connect(), ESP_OK);
}

TEST_F(ProvisioningTest, failAndRecover) {
    EXPECT_CALL(event_group, wait(success_bit | fail_bit))
        .Times(2)
        .WillOnce(Return(fail_bit))
        .WillOnce(Return(success_bit));
    EXPECT_CALL(event_group, wait(end_bit)).WillOnce(Return(end_bit));
    EXPECT_EQ(prov.connect(), ESP_OK);
}
