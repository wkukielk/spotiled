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