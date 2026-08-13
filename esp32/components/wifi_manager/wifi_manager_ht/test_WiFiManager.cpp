#include "MockTime.hpp"
#include "TestableWiFiManager.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace std;
using namespace testing;

namespace {
enum class Provisioned { True, False };
}

using namespace wifi;

class ProvisionedWiFiManagerTest : public Test {
  protected:
    TimeHandle time_handle{mkuptr<NiceMock<MockTime>>()};

    NiceMock<MockHandlerFactory> factory{};
    TestableProvisionedWiFiManager manager{
        [&](auto &egroup) { return factory.call(egroup); }};

    NiceMock<MockWiFi> &wifi_ = manager.get_wifi();
    optional<MockProvisioningIsProvisioned> &provisioning =
        manager.get_provisioning();
    MockEventGroup &egroup = manager.get_event_group();

    void SetUp() override {
        EXPECT_CALL(egroup, wait(end_bit)).Times(AnyNumber());
    }
};

TEST_F(ProvisionedWiFiManagerTest, constructDestruct) {
    EXPECT_CALL(egroup, wait(end_bit)).WillOnce(Return(end_bit));
    EXPECT_CALL(wifi_, stop()).WillOnce(Return(0));
}

TEST_F(ProvisionedWiFiManagerTest, initSuccess) {
    EXPECT_CALL(wifi_, set_mode(WiFiMode::STA)).WillOnce(Return(0));
    manager.init();
    ASSERT_FALSE(provisioning.has_value());
}

TEST_F(ProvisionedWiFiManagerTest, initFail) {
    EXPECT_CALL(wifi_, set_mode(WiFiMode::STA)).WillOnce(Return(-1));
    ASSERT_THROW(manager.init(), InitError);
}

TEST_F(ProvisionedWiFiManagerTest, isConnected) {
    EXPECT_CALL(egroup, get()).WillOnce(Return(wifi::success_bit));
    EXPECT_TRUE(manager.is_connected());
}

TEST_F(ProvisionedWiFiManagerTest, isConnectedNo) {
    EXPECT_CALL(egroup, get()).WillOnce(Return(0));
    EXPECT_FALSE(manager.is_connected());
}

TEST_F(ProvisionedWiFiManagerTest, connectSuccess) {
    manager.init();
    EXPECT_CALL(wifi_, start()).WillOnce(Return(ESP_OK));
    EXPECT_CALL(egroup, wait(success_bit | fail_bit))
        .WillOnce(Return(success_bit));
    EXPECT_CALL(egroup, wait(end_bit)).WillOnce(Return(end_bit));
    ASSERT_EQ(manager.connect(), ESP_OK);
    ASSERT_FALSE(provisioning.has_value());
}

TEST_F(ProvisionedWiFiManagerTest, connectFailWiFiStart) {
    manager.init();
    EXPECT_CALL(wifi_, start()).WillOnce(Return(ESP_FAIL));
    ASSERT_THROW(manager.connect(), WiFiManagerError);
}

TEST_F(ProvisionedWiFiManagerTest, connectFail) {
    manager.init();
    EXPECT_CALL(wifi_, start()).WillOnce(Return(ESP_OK));
    EXPECT_CALL(egroup, wait(success_bit | fail_bit))
        .WillOnce(Return(fail_bit));
    EXPECT_CALL(egroup, wait(end_bit)).WillOnce(Return(end_bit));
    ASSERT_EQ(manager.connect(), ESP_FAIL);
}

class NotProvisionedWiFiManagerTest : public Test {
  protected:
    TimeHandle time_handle{mkuptr<MockTime>()};
    NiceMock<MockHandlerFactory> factory{};
    TestableNotProvisionedWiFiManager manager{
        [&](auto &egroup) { return factory.call(egroup); }};
    MockWiFi &wifi_ = manager.get_wifi();
    optional<MockProvisioningNotProvisioned> &provisioning =
        manager.get_provisioning();
    MockEventGroup &egroup = manager.get_event_group();

    void SetUp() override {
        EXPECT_CALL(egroup, wait(end_bit)).Times(AnyNumber());
    }
};

TEST_F(NotProvisionedWiFiManagerTest, initSuccess) {
    EXPECT_CALL(wifi_, set_mode(WiFiMode::STA)).WillOnce(Return(0));
    manager.init();
    ASSERT_TRUE(provisioning.has_value());
}

TEST_F(NotProvisionedWiFiManagerTest, connectSuccess) {
    manager.init();
    EXPECT_CALL(*provisioning, connect()).WillOnce(Return(ESP_OK));
    ASSERT_EQ(manager.connect(), ESP_OK);
    ASSERT_FALSE(provisioning.has_value());
}

TEST_F(NotProvisionedWiFiManagerTest, connectFail) {
    manager.init();
    EXPECT_CALL(*provisioning, connect()).WillOnce(Return(ESP_FAIL));
    ASSERT_EQ(manager.connect(), ESP_FAIL);
    ASSERT_TRUE(provisioning.has_value());
}
