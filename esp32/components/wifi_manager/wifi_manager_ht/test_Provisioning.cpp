#include "BaseTest.hpp"
#include "I_WiFi.hpp"
#include "MockEventGroup.hpp"
#include "MockEventHandler.hpp"
#include "MockWiFiProv.hpp"
#include "Provisioning.hpp"
#include "esp_event.h"
#include "wifi_aux.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;

class ProvisioningTest : public BaseTest {
  protected:
    ProvisioningTest() {
        event_group = std::make_unique<MockEventGroup>();
        manager = std::make_unique<Provisioning>(get_essentials(), *event_group,
                                                 service_name);
        manager->on();
        wifi_prov = (MockWiFiProv *)&manager->get_wifi_prov();
    }

    ~ProvisioningTest() { manager->off(); }

    ProvEssentials get_essentials() {
        return {.get_wifi_prov = []() { return new MockWiFiProv{}; },
                .get_handler =
                    [](util::I_EventGroup &g) {
                        (void)g;
                        return new MockEventHandler{};
                    }};
    }

    MockWiFiProv *wifi_prov{};
    std::unique_ptr<MockEventGroup> event_group;
    std::unique_ptr<Provisioning> manager;
    const char *service_name = "MAC_HERE";
};

using wifi::end_bit;
using wifi::fail_bit;
using wifi::success_bit;

TEST_F(ProvisioningTest, isProvisioned) {
    EXPECT_CALL(*wifi_prov, is_provisioned(_)).WillOnce(Return(ESP_FAIL));
    EXPECT_THROW(manager->is_provisioned(), WiFiManagerError);
}

TEST_F(ProvisioningTest, success) {
    EXPECT_CALL(*wifi_prov, start_provisioning(_, _, StrEq(service_name), _));
    EXPECT_CALL(*event_group, wait(success_bit | fail_bit))
        .WillOnce(Return(success_bit));
    EXPECT_EQ(manager->connect(), ESP_OK);
}

TEST_F(ProvisioningTest, failAndRecover) {
    EXPECT_CALL(*event_group, wait(success_bit | fail_bit))
        .Times(2)
        .WillOnce(Return(fail_bit))
        .WillOnce(Return(success_bit));
    EXPECT_CALL(*event_group, wait(end_bit)).WillOnce(Return(end_bit));
    EXPECT_EQ(manager->connect(), ESP_OK);
}
