#ifndef MY_TEST_UTILS_HPP
#define MY_TEST_UTILS_HPP

#include "esp_err.h"

esp_err_t enable_leak_check();

esp_err_t disable_leak_check();

esp_err_t set_leak_threshold(size_t threshold);

#endif
