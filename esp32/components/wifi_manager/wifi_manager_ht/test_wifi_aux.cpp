#include "BaseTest.hpp"
#include "wifi_aux.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::StrEq;

class WiFiAuxTest : public BaseTest {};

TEST_F(WiFiAuxTest, resolve) {
    esp_ip4_addr_t addr{.addr = 0x2001A8C0};
    EXPECT_CALL(*mdns, query_a(StrEq("my-computer"), _, _))
        .WillOnce(DoAll(SetArgPointee<2>(addr), Return(ESP_OK)));
    ASSERT_EQ(mdns_resolve("my-computer.local"), "192.168.1.32");
}
