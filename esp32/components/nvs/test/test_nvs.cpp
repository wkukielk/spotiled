#include "global.hpp"
#include "mytest_utils.hpp"
#include "nvs.hpp"
#include "unity.h"

namespace {
std::unique_ptr<NVS> handle;

void setup(void) { handle = std::make_unique<NVS>(); }

void teardown(void) { handle.reset(); }

}; // namespace

TEST_CASE("setup", "[nvs]") {
    disable_leak_check();
    setup();
}

TEST_CASE("write read", "[nvs]") {
    enable_leak_check();
    handle->write("test", "42");
    TEST_ASSERT_EQUAL_STRING(handle->read("test").c_str(), "42");
}

TEST_CASE("read nonexistent", "[nvs]") {
    TEST_ASSERT_EQUAL_STRING(handle->read("nonex").c_str(), "");
}

TEST_CASE("erase", "[nvs]") {
    handle->write("test", "43");
    TEST_ASSERT_EQUAL(ESP_OK, handle->erase());
    TEST_ASSERT_EQUAL_STRING(handle->read("test").c_str(), "");
}

TEST_CASE("teardown", "[nvs]") { teardown(); }
