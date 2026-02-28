#include "WiFiManager.hpp"
#include "WiFiEventHandler.hpp"
#include "esp_log.h"
#include "global.hpp"
#include "wifi_aux.hpp"

using util::end_bit;
using util::fail_bit;
using util::success_bit;

static const char *TAG = "WiFiManager";

WiFiManager::WiFiManager(std::unique_ptr<util::I_EventGroup> &&event_group,
                         std::unique_ptr<I_ToggleEventHandler> &&handler,
                         std::unique_ptr<I_Provisioning> &&provisioning)
    : event_group{std::move(event_group)}, handler{std::move(handler)},
      provisioning{std::move(provisioning)} {}

WiFiManager::~WiFiManager() {
    wifi().stop();
    event_group->wait(end_bit);
}

int WiFiManager::init() {
    UTIL_THROW(wifi().set_mode(WiFiMode::STA), InitError,
               "Failed to set STA mode");
    switch_to_prov();
    if (provisioning->is_provisioned()) {
        ESP_LOGI(TAG, "ESP is provisioned");
        switch_to_no_prov();
    } else
        ESP_LOGI(TAG, "ESP isn't provisioned");
    return ESP_OK;
}

int WiFiManager::connect() {
    return provisioning->is_on() ? connect_with_prov() : connect_without_prov();
}

int WiFiManager::disconnect() {
    UTIL_RETURN(wifi().stop());
    auto bits = event_group->wait(end_bit, pdFALSE, pdFALSE,
                                  10000 / portTICK_PERIOD_MS);
    return bits & end_bit ? ESP_OK : ESP_FAIL;
}

bool WiFiManager::is_connected() { return event_group->get() & success_bit; }

int WiFiManager::connect_with_prov() {
    auto status = provisioning->connect();
    if (status == ESP_OK)
        switch_to_no_prov();
    return status;
}

int WiFiManager::connect_without_prov() {
    UTIL_LOGI("Connecting...");
    UTIL_THROW(wifi().start(), WiFiManagerError, "WiFi start");
    return wifi::await_connection(*event_group);
}

void WiFiManager::switch_to_prov() {
    provisioning->on();
    handler->off();
}

void WiFiManager::switch_to_no_prov() {
    handler->on();
    provisioning->off();
}
