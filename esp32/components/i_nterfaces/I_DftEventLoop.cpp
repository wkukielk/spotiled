#include "I_DftEventLoop.hpp"
#include "errors.hpp"

DftEventLoopConfigurer::DftEventLoopConfigurer() {
    UTIL_THROW(DftEventLoop::create(), InitError, "DftEventLoop init");
}

DftEventLoopConfigurer::~DftEventLoopConfigurer() {
    auto err = DftEventLoop::cleanup();
    if (err != ESP_OK)
        ESP_LOGW("DftEventLoop", "DftEventLoop cleanup fail: %d", err);
}

std::optional<DftEventLoop> DftEventLoop::instance{};

esp_err_t DftEventLoop::create() noexcept {
    UTIL_RETURN(esp_event_loop_create_default());
    DftEventLoop::instance.emplace();
    return ESP_OK;
}

esp_err_t DftEventLoop::cleanup() noexcept {
    DftEventLoop::instance.reset();
    return esp_event_loop_delete_default();
}