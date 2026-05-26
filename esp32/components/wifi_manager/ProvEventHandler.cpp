#include "ProvEventHandler.hpp"
#include "I_WiFi.hpp"
#include "I_WiFiProv.hpp"
#include "errors.hpp"
#include "esp_log.h"
#include "esp_wifi.h"

static const char *TAG = "ProvEventHandler";
util::I_EventGroup *ProvEventHandler::event_group{};
uint8_t ProvEventHandler::retries{};
int8_t ProvEventHandler::retries_left{};
esp_event_handler_instance_t ProvEventHandler::disconnect_handle{};
esp_event_handler_instance_t ProvEventHandler::ip_handle{};
esp_event_handler_instance_t ProvEventHandler::handle{};
const util::cbdata ProvEventHandler::cbdata[2]{
    {.callback = handler, .base = NETWORK_PROV_EVENT, .id = ESP_EVENT_ANY_ID},
    {.callback = disconnect_handler,
     .base = WIFI_EVENT,
     .id = WIFI_EVENT_STA_DISCONNECTED}};
ProvReason ProvEventHandler::reason = ProvReason::OK;

ProvEventHandler::ProvEventHandler(util::I_EventGroup &event_group,
                                   uint8_t retries, util::Register reg) {
    this->retries = retries;
    UTIL_THROW(ProvEventHandler::event_group != nullptr, InitError,
               "two prov handlers");
    ProvEventHandler::event_group = &event_group;
    event_group.set(util::end_bit);
    if (reg == util::Register::YES)
        UTIL_THROW(register_self(), InitError, "ProvEventHandler register");
    ESP_LOGI("ProvEventHandler", "init");
}

ProvEventHandler::~ProvEventHandler() {
    esp_event_handler_instance_unregister(
        WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, disconnect_handle);
    esp_event_handler_instance_unregister(NETWORK_PROV_EVENT, ESP_EVENT_ANY_ID,
                                          handle);
    ProvEventHandler::event_group = nullptr;
    ESP_LOGI("ProvEventHandler", "deinit");
}

esp_err_t ProvEventHandler::register_self() {
    UTIL_RETURN(esp_event_handler_instance_register(
        NETWORK_PROV_EVENT, ESP_EVENT_ANY_ID, &handler, NULL, &handle));
    UTIL_RETURN(esp_event_handler_instance_register(
        WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, NULL,
        &ip_handle));
    return ESP_OK;
}

std::span<const util::cbdata> ProvEventHandler::get_cbdata() const noexcept {
    return straux::to_span(cbdata, 2);
}

ProvReason ProvEventHandler::get_reason() const noexcept { return reason; }

void ProvEventHandler::handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
    switch (event_id) {
    case (int32_t)ProvEvent::START:
        retries_left = retries;
        event_group->clear(util::end_bit | util::fail_bit | util::success_bit);
        reason = ProvReason::OK;
        break;
    case (int32_t)ProvEvent::SET_STA_CONFIG: {
        auto *config = static_cast<wifi_config_t *>(event_data);
        config->sta = {
            .threshold = {.authmode = WIFI_AUTH_WPA2_PSK},
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        };
        break;
    }
    case (int32_t)ProvEvent::CRED_SUCCESS:
        event_group->set(util::success_bit);
        break;
    case (int32_t)ProvEvent::CRED_FAIL: {
        reason = *static_cast<ProvReason *>(event_data);
        ESP_LOGE(TAG,
                 "Provisioning failed!\n\tReason : %s"
                 "\n\tPlease reset to factory and retry provisioning",
                 (reason == ProvReason::AUTH_ERROR)
                     ? "Wi-Fi station authentication failed"
                     : "Wi-Fi access-point not found");
        event_group->set(util::fail_bit || util::end_bit);
        break;
    }
    case (int32_t)ProvEvent::END:
        ESP_LOGI(TAG, "Provisioning ended");
        event_group->set(util::end_bit);
        break;
    default:;
    }
}

void ProvEventHandler::disconnect_handler(void *arg,
                                          esp_event_base_t event_base,
                                          int32_t event_id, void *event_data) {
    auto *data = static_cast<wifi_event_sta_disconnected_t *>(event_data);
    auto reason = static_cast<WiFiReason>(data->reason);
    if (I_WiFi::is_disconnect_recoverable(reason) && --retries_left <= 0) {
        ESP_LOGI(TAG, "Too many retries. Failure");
        // wifi_prov_mgr's event handler will receive modified event_data.
        // The following reason will cause wifi_prov_mgr to stop retrying.
        data->reason = WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT;
    }
}
