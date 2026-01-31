#include "esp_log.h"
#include "nvs.hpp"
#include "unity.h"

TEST_CASE("write read", "[nvs]") {
    NVS::write("test", 42);
    int read = 0;
    TEST_ASSERT_EQUAL(0, NVS::read("test", read));
    TEST_ASSERT_EQUAL(42, read);
}

TEST_CASE("read nonexistent", "[nvs]") {
    int read = 0;
    TEST_ASSERT_EQUAL(ESP_ERR_NVS_NOT_FOUND, NVS::read("nonex", read));
    TEST_ASSERT_EQUAL(0, read);
}

TEST_CASE("erase", "[nvs]") {
    NVS::write("test", 42);
    TEST_ASSERT_EQUAL(ESP_OK, NVS::erase());
    int read = 0;
    TEST_ASSERT_EQUAL(ESP_ERR_NVS_NOT_FOUND, NVS::read("test", read));
}

extern "C" void app_main(void) {
    NVS::init();

    UNITY_BEGIN();
    unity_run_all_tests();
    UNITY_END();
}
