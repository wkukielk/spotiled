#include "BaseTest.hpp"
#include "wifi_aux.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace std;
using namespace testing;

class WiFiAuxTest : public BaseTest {
  protected:
    esp_ip4_addr_t addr{.addr = 0x2001A8C0};
};

TEST_F(WiFiAuxTest, resolveHostname) {
    EXPECT_CALL(*mdns_m, query_a(StrEq("my-computer"), _, _))
        .WillOnce(DoAll(SetArgPointee<2>(addr), Return(ESP_OK)));
    auto ip_str = std::string{};
    ASSERT_EQ(mdns_resolve("my-computer.local", ip_str), ESP_OK);
    ASSERT_EQ(ip_str, "192.168.1.32");
}

TEST_F(WiFiAuxTest, resolveHostnameNoDotLocal) {
    EXPECT_CALL(*mdns_m, query_a(StrEq("my-computer"), _, _))
        .WillOnce(DoAll(SetArgPointee<2>(addr), Return(ESP_OK)));
    auto ip_str = std::string{};
    ASSERT_EQ(mdns_resolve("my-computer.local", ip_str), ESP_OK);
    ASSERT_EQ(ip_str, "192.168.1.32");
}

TEST_F(WiFiAuxTest, resolveHostnameTimeout) {
    EXPECT_CALL(*mdns_m, query_a(StrEq("my-computer"), 551, _))
        .WillOnce(Return(ESP_OK));
    auto ip_str = std::string{};
    ASSERT_EQ(mdns_resolve("my-computer.local", ip_str, 551), ESP_OK);
}

TEST_F(WiFiAuxTest, resolveUrl) {
    EXPECT_CALL(*mdns_m, query_a(StrEq("my-computer"), _, _))
        .WillOnce(DoAll(SetArgPointee<2>(addr), Return(ESP_OK)));
    auto url = URL{"http://my-computer.local:8000/users?age_gt=31"};
    auto err = esp_err_t{};
    auto resolved = resolve_url(url, ref(err));
    ASSERT_EQ(err, ESP_OK);
    ASSERT_EQ(resolved->get(), "http://192.168.1.32:8000/users?age_gt=31");
}

TEST_F(WiFiAuxTest, resolveUrlAlreadyResolved) {
    auto url = URL{"http://my-computer:8000/users?age_gt=31"};
    auto err = esp_err_t{};
    ASSERT_EQ(resolve_url(url, ref(err))->get(), url.get());
}

TEST_F(WiFiAuxTest, resolveUrlTimeout) {
    EXPECT_CALL(*mdns_m, query_a(StrEq("my-computer"), 531, _))
        .WillOnce(Return(ESP_OK));
    auto url = URL{"http://my-computer.local:8000/users?age_gt=31"};
    auto err = esp_err_t{};
    resolve_url(url, ref(err), 531);
}

TEST_F(WiFiAuxTest, resolveFail) {
    EXPECT_CALL(*mdns_m, query_a(StrEq("my-computer"), _, _))
        .WillOnce(Return(ESP_ERR_NOT_FOUND));
    auto url = URL{"http://my-computer.local:8000/users?age_gt=31"};
    auto err = esp_err_t{};
    auto resolved = resolve_url(url, ref(err));
    ASSERT_FALSE(resolved.has_value());
    ASSERT_EQ(err, ESP_ERR_NOT_FOUND);
}