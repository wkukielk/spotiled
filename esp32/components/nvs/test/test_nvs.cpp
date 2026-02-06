#include "esp_log.h"
#include "nvs.h"
#include "nvs.hpp"
#include "unity.h"
#include "unity_fixture_extras.h"

namespace {
std::unique_ptr<NVS> nvs;
void setup() { nvs = std::make_unique<NVS>(); }
void teardown() { nvs.reset(); }
}; // namespace

TEST_CASE("write read", "[nvs]") {
    setup();
    nvs->write("test", "42");
    auto read = CharBuf{};
    TEST_ASSERT_EQUAL(0, nvs->read("test", read));
    TEST_ASSERT_EQUAL_STRING("42", read.data());
}

TEST_CASE("read nonexistent", "[nvs]") {
    auto read = CharBuf{};
    TEST_ASSERT_EQUAL(ESP_ERR_NVS_NOT_FOUND, nvs->read("nonex", read));
    TEST_ASSERT(read.empty());
}

TEST_CASE("erase", "[nvs]") {
    nvs->write("test", "42");
    TEST_ASSERT_EQUAL(ESP_OK, nvs->erase());
    auto read = CharBuf{};
    TEST_ASSERT_EQUAL(ESP_ERR_NVS_NOT_FOUND, nvs->read("test", read));
    teardown();
}
