#include "memory_checks.h"
#include "nvs.hpp"
#include "unity.h"
#include "unity_fixture_extras.h"

namespace {
std::unique_ptr<NVS> handle;

void setup(void) { handle = std::make_unique<NVS>(); }

void teardown(void) { handle.reset(); }

}; // namespace

TEST_CASE("setup", "[nvs]") {
    test_utils_set_leak_level(100000, ESP_LEAK_TYPE_CRITICAL,
                              ESP_COMP_LEAK_GENERAL);
    setup();
}

TEST_CASE("write read", "[nvs]") {
    test_utils_set_leak_level(0, ESP_LEAK_TYPE_CRITICAL, ESP_COMP_LEAK_GENERAL);
    handle->write("test", "42");
    TEST_ASSERT_EQUAL_STRING(handle->read("test").c_str(), "42");
}

TEST_CASE("read nonexistent", "[nvs]") {
    TEST_ASSERT_EQUAL_STRING(handle->read("nonex").c_str(), "");
}

TEST_CASE("erase", "[nvs]") {
    handle->write("test", "42");
    TEST_ASSERT_EQUAL(ESP_OK, handle->erase());
    TEST_ASSERT_EQUAL_STRING(handle->read("test").c_str(), "");
}

TEST_CASE("teardown", "[nvs]") { teardown(); }
