#include "ToggleEventHandler.hpp"
#include "esp_log.h"

ToggleEventHandler::ToggleEventHandler(
    std::unique_ptr<util::EventHandler> &&handler)
    : handler{std::move(handler)} {
    auto cbd = this->handler->get_cbdata();
    cbdata = std::vector<util::cbdata>{cbd.data(), cbd.data() + cbd.size()};
}

ToggleEventHandler::~ToggleEventHandler() noexcept { off(); }

esp_err_t ToggleEventHandler::on() {
    ESP_LOGI("ToggleEventHandler", "on");
    for (auto &cbd : cbdata) {
        auto instance = register_handler(cbd);
        UTIL_RETURN(instance == nullptr);
        cbd.instance = instance;
    }
    return ESP_OK;
}

esp_err_t ToggleEventHandler::off() {
    for (auto &cbd : cbdata) {
        UTIL_RETURN(esp_event_handler_instance_unregister(cbd.base, cbd.id,
                                                          cbd.instance));
        cbd.instance = nullptr;
    }
    return ESP_OK;
    ESP_LOGI("ToggleEventHandler", "off");
}

bool ToggleEventHandler::is_on() const {
    return !cbdata.empty() && cbdata[0].instance;
}

esp_event_handler_instance_t
ToggleEventHandler::register_handler(const util::cbdata &cbdata) {
    esp_event_handler_instance_t handle{};
    esp_event_handler_instance_register(cbdata.base, cbdata.id, cbdata.callback,
                                        (void *)&cbdata, &handle);
    return handle;
}