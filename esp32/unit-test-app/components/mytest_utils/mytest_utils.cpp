#include "memory_checks.h"
#include <stdio.h>

void enable_leak_check() {
    test_utils_set_leak_level(0, ESP_LEAK_TYPE_CRITICAL, ESP_COMP_LEAK_GENERAL);
}

void disable_leak_check() {
    test_utils_set_leak_level(100000, ESP_LEAK_TYPE_CRITICAL,
                              ESP_COMP_LEAK_GENERAL);
}

void set_leak_threshold(size_t threshold) {
    test_utils_set_leak_level(threshold, ESP_LEAK_TYPE_CRITICAL,
                              ESP_COMP_LEAK_GENERAL);
}
