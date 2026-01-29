#include "esp_log.h"
#include "hello.hpp"
#include <stdio.h>

extern "C" void app_main(void) { ESP_LOGI("MAIN", "%s", hello()); }
