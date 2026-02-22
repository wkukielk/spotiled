#include "BaseTest.hpp"
#include "wifi_aux.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::StrEq;

class WiFiAuxTest : public BaseTest {
  protected:
    esp_ip4_addr_t addr{.addr = 0x2001A8C0};
};

TEST_F(WiFiAuxTest, resolveHostname) {
    EXPECT_CALL(*mdns, query_a(StrEq("my-computer"), _, _))
        .WillOnce(DoAll(SetArgPointee<2>(addr), Return(ESP_OK)));
    ASSERT_EQ(mdns_resolve("my-computer.local"), "192.168.1.32");
}

TEST_F(WiFiAuxTest, resolveHostnameNoDotLocal) {
    EXPECT_CALL(*mdns, query_a(StrEq("my-computer"), _, _))
        .WillOnce(DoAll(SetArgPointee<2>(addr), Return(ESP_OK)));
    ASSERT_EQ(mdns_resolve("my-computer"), "192.168.1.32");
}

TEST_F(WiFiAuxTest, resolveUrl) {
    EXPECT_CALL(*mdns, query_a(StrEq("my-computer"), _, _))
        .WillOnce(DoAll(SetArgPointee<2>(addr), Return(ESP_OK)));
    auto url = URL{"http://my-computer.local:8000/users?age_gt=31"};
    auto resolved = resolve_url(url);
    ASSERT_EQ(resolved.get(), "http://192.168.1.32:8000/users?age_gt=31");
}

TEST_F(WiFiAuxTest, resolveUrlAlreadyResolved) {
    auto url = URL{"http://my-computer:8000/users?age_gt=31"};
    ASSERT_EQ(resolve_url(url).get(), url.get());
}