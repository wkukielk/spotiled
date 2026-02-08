#include "ToggleEventHandler.hpp"
#include "esp_log.h"

ToggleEventHandler::ToggleEventHandler(
    std::function<std::unique_ptr<util::EventHandler>()> get_handler)
    : get_handler{get_handler} {}

ToggleEventHandler::~ToggleEventHandler() noexcept { off(); }

esp_err_t ToggleEventHandler::on() {
    ESP_LOGI("ToggleEventHandler", "on");
    handler = get_handler();
    return ESP_OK;
}

esp_err_t ToggleEventHandler::off() {
    ESP_LOGI("ToggleEventHandler", "off");
    handler.reset();
    return ESP_OK;
}

bool ToggleEventHandler::is_on() const { return handler.get(); }
