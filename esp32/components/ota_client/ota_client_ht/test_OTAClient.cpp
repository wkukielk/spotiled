#include "MockOTA.hpp"
#include "MockPartition.hpp"
#include "TestableOTAClient.hpp"
#include <gtest/gtest.h>

using namespace std;
using namespace testing;

class OTAClientTest : public testing::Test {
  protected:
    OTAClientTest() : ota_client{get_cfg()}, ota{&ota_client.get_ota()} {}

    static ota::HTTPSConfig get_cfg() {
        return ota::HTTPSConfig{.url = "https://backend.pl/app",
                                .cert_pem = "",
                                .cert_len = 0,
                                .skip_cn_check = true};
    }

    TestableOTAClient ota_client;
    MockOTA *ota{};

    void expect_invalid_partition(const char *version) {
        auto desc = esp_app_desc_t{};
        strcpy(desc.version, version);
        auto part = mkuptr<MockPartition>();
        EXPECT_CALL(*part, get_description(_))
            .WillOnce(DoAll(SetArgPointee<0>(desc), Return(ESP_OK)));
        EXPECT_CALL(*ota, get_other_partition()).WillOnce(Return(move(part)));
    }

    void expect_no_invalid_partition() {
        EXPECT_CALL(*ota, get_other_partition()).WillOnce(Return(nullptr));
    }
};

TEST_F(OTAClientTest, get_version) {
    auto desc = esp_app_desc_t{};
    strcpy(desc.version, "1.0");

    auto part = mkuptr<MockPartition>();
    EXPECT_CALL(*part, get_description(_))
        .WillOnce(DoAll(SetArgPointee<0>(desc), Return(ESP_OK)));
    EXPECT_CALL(*ota, get_running_partition()).WillOnce(Return(move(part)));
    auto version = ota_client.get_app_version();
    ASSERT_EQ(version, "1.0");
}

TEST_F(OTAClientTest, noInvalidVersion) {
    expect_no_invalid_partition();
    ASSERT_EQ(ota_client.get_invalid_version(), nullopt);
}

TEST_F(OTAClientTest, invalidVersion) {
    expect_invalid_partition("1.3");
    ASSERT_EQ(ota_client.get_invalid_version(), "1.3");
}

TEST_F(OTAClientTest, invalidVersionDescriptionFails) {
    auto part = mkuptr<MockPartition>();
    EXPECT_CALL(*part, get_description(_)).WillOnce(Return(ESP_FAIL));
    EXPECT_CALL(*ota, get_other_partition()).WillOnce(Return(move(part)));
    ASSERT_EQ(ota_client.get_invalid_version(), nullopt);
}

TEST_F(OTAClientTest, updateUrlWithoutBlacklist) {
    expect_no_invalid_partition();
    ASSERT_EQ(ota_client.get_update_url(), "https://backend.pl/app");
}

TEST_F(OTAClientTest, updateUrlBlacklistsInvalidVersion) {
    expect_invalid_partition("1.3");
    ASSERT_EQ(ota_client.get_update_url(),
              "https://backend.pl/app?blacklist=1.3");
}

TEST(OTAClientSkipBlacklistTest, updateUrlIgnoresInvalidVersion) {
    auto cfg = ota::HTTPSConfig{.url = "https://backend.pl/app",
                                .cert_pem = "",
                                .cert_len = 0,
                                .skip_cn_check = true,
                                .skip_blacklist = true};
    TestableOTAClient client{move(cfg)};
    EXPECT_CALL(client.get_ota(), get_other_partition()).Times(0);
    ASSERT_EQ(client.get_update_url(), "https://backend.pl/app");
}

TEST_F(OTAClientTest, updateRequestsBlacklistedUrl) {
    expect_invalid_partition("1.3");
    auto requested = string{};
    EXPECT_CALL(*ota, update(_))
        .WillOnce([&](const ota::config *cfg) {
            requested = cfg->http_config->url;
            return ESP_OK;
        });
    ASSERT_EQ(ota_client.update(), ESP_OK);
    ASSERT_EQ(requested, "https://backend.pl/app?blacklist=1.3");
}

TEST_F(OTAClientTest, updatePassesNotModifiedThrough) {
    expect_no_invalid_partition();
    EXPECT_CALL(*ota, update(_)).WillOnce(Return(ESP_ERR_HTTP_NOT_MODIFIED));
    ASSERT_EQ(ota_client.update(), ESP_ERR_HTTP_NOT_MODIFIED);
}