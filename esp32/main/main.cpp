#include "app.hpp"
#include "esp_log.h"
#include <stdio.h>

extern "C" void app_main(void) {
    try {
        auto *app = App::create();
        app->run();
    } catch (const std::runtime_error &e) {
        ESP_LOGE("MAIN", "%s", e.what());
        ESP_LOGE("MAIN", "App init failed.");
        return;
    } catch (const std::exception &e) {
        ESP_LOGE("MAIN", "%s", e.what());
    } catch (...) {
        ESP_LOGE("MAIN", "unknown exception");
    }
    ESP_LOGI("MAIN", "main return");
}
