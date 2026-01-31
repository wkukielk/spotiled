#include "app.hpp"
#include "esp_log.h"
#include <stdio.h>

extern "C" void app_main(void) {
    try {
        auto *app = new App{};
        app->run();
    } catch (const std::runtime_error &e) {
        ESP_LOGE("MAIN", "%s", e.what());
        ESP_LOGE("MAIN", "App init failed.");
        return;
    } catch (...) {
        ESP_LOGI("MAIN", "fatal exception");
    }
    ESP_LOGI("MAIN", "main return");
}
