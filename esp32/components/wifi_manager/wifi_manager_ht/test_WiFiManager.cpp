#include "BaseTest.hpp"
#include "MockEventGroup.hpp"
#include "MockEventHandler.hpp"
#include "MockProvisioning.hpp"
#include "MockToggleEventHandler.hpp"
#include "MockWiFi.hpp"
#include "MockWiFiProv.hpp"
#include "WiFiManager.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

namespace {
enum class Provisioned { True, False };
}

class WiFiManagerTest : public BaseTest {
  protected:
    WiFiManagerTest() {}

    void init(Provisioned provisioned) {
        auto event_group = std::make_unique<MockEventGroup>();
        auto handler = std::make_unique<MockToggleEventHandler>();
        auto provisioning = std::make_unique<MockProvisioning>();
        this->event_group = event_group.get();
        this->handler = handler.get();
        this->provisioning = provisioning.get();
        manager = std::make_unique<WiFiManager>(std::move(event_group),
                                                std::move(handler),
                                                std::move(provisioning));
        bool is_prov = provisioned == Provisioned::True;
        EXPECT_CALL(*this->provisioning, is_provisioned())
            .WillOnce(Return(is_prov));
        UTIL_CHECK(manager->init(), "WifiManager init fail");
    }

    MockEventGroup *event_group{};
    MockToggleEventHandler *handler{};
    MockProvisioning *provisioning{};
    std::unique_ptr<WiFiManager> manager;
    const char *service_name = "MAC_HERE";
};

using util::end_bit;
using util::fail_bit;
using util::success_bit;

TEST_F(WiFiManagerTest, init) {
    init(Provisioned::False);
    EXPECT_CALL(*event_group, get()).WillOnce(Return(end_bit));
    EXPECT_FALSE(manager->is_connected());
}

TEST_F(WiFiManagerTest, connectWithProvSuccess) {
    init(Provisioned::False);
    EXPECT_CALL(*provisioning, is_on()).WillOnce(Return(true));
    EXPECT_CALL(*provisioning, connect()).WillOnce(Return(ESP_OK));
    EXPECT_CALL(*handler, on());
    EXPECT_CALL(*provisioning, off());
    EXPECT_EQ(manager->connect(), ESP_OK);
}

TEST_F(WiFiManagerTest, connectWithProvFail) {
    init(Provisioned::False);
    EXPECT_CALL(*provisioning, is_on()).WillOnce(Return(true));
    EXPECT_CALL(*provisioning, connect()).WillOnce(Return(ESP_FAIL));
    EXPECT_EQ(manager->connect(), ESP_FAIL);
}