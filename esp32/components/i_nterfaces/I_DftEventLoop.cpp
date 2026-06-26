#include "I_DftEventLoop.hpp"
#include "errors.hpp"

DftEventLoop::DftEventLoop() {
    UTIL_THROW(esp_event_loop_create_default(), InitError, "DftEventLoop init");
}

DftEventLoop::~DftEventLoop() noexcept {
    auto err = esp_event_loop_delete_default();
    if (err != ESP_OK)
        ESP_LOGW("DftEventLoop", "DftEventLoop cleanup fail: %d", err);
}
