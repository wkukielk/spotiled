#include "OTA.hpp"
#include "mytest_utils.hpp"
#include "unity.h"

using namespace std;

namespace {

void setup() {}

void teardown() {}

}; // namespace

TEST_CASE("setup", "[ota_client]") {
    setup();
    disable_leak_check();
}

TEST_CASE("get_description", "[ota_client]") {
    enable_leak_check();
    auto ota = OTA{};
    auto partition = ota.get_running_partition();
    esp_app_desc_t desc;
    partition->get_description(&desc);
    TEST_ASSERT_EQUAL_STRING((char *)desc.version, "1.0");
}

TEST_CASE("teardown", "[ota_client]") { teardown(); }
