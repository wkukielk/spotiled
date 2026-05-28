#include "OTAClient.hpp"
#include "OTAStub.hpp"
#include "global.hpp"
#include "nvs.hpp"
#include "unity.h"

namespace {
ota::HTTPSConfig cfg{.url = "example.com/binary",
                     .auth_header = "secret",
                     .cert_pem = "cert",
                     .cert_len = 5,
                     .skip_cn_check = true};
}

TEST_CASE("get_app_version", "[ota]") {
    auto client = OTAClient<OTAStub>{std::move(cfg)};
    TEST_ASSERT_TRUE(client.get_app_version() == "version");
}

TEST_CASE("update", "[ota]") {
    auto client = OTAClient<OTAStub>{std::move(cfg)};
    TEST_ASSERT_TRUE(client.update());
}

extern "C" void app_main(void) {
    UNITY_BEGIN();
    unity_run_tests_by_tag("[ota]", false);
    UNITY_END();
}
