#include "I_WiFi.hpp"
#include "I_WiFiProv.hpp"
#include "ProvEventHandler.hpp"
#include "errors.hpp"
#include "esp_log.h"
#include "esp_wifi.h"
#include "wifi_aux.hpp"

template <util::I_EventGroup EGroup, DftEventLoopHandle ELoop>
EGroup *ProvEventHandler<EGroup, ELoop>::event_group{};

template <util::I_EventGroup EGroup, DftEventLoopHandle ELoop>
uint8_t ProvEventHandler<EGroup, ELoop>::retries{};

template <util::I_EventGroup EGroup, DftEventLoopHandle ELoop>
int8_t ProvEventHandler<EGroup, ELoop>::retries_left{};

template <util::I_EventGroup EGroup, DftEventLoopHandle ELoop>
esp_event_handler_instance_t
    ProvEventHandler<EGroup, ELoop>::disconnect_handle{};

template <util::I_EventGroup EGroup, DftEventLoopHandle ELoop>
esp_event_handler_instance_t ProvEventHandler<EGroup, ELoop>::handle{};

template <util::I_EventGroup EGroup, DftEventLoopHandle ELoop>
ProvReason ProvEventHandler<EGroup, ELoop>::reason = ProvReason::OK;

template <util::I_EventGroup EGroup, DftEventLoopHandle ELoop>
ProvEventHandler<EGroup, ELoop>::ProvEventHandler(EGroup &event_group,
                                                  uint8_t retries) {
    this->retries = retries;
    UTIL_THROW(ProvEventHandler::event_group != nullptr, InitError,
               "two prov handlers");
    ProvEventHandler::event_group = &event_group;
    event_group.set(wifi::end_bit);
    UTIL_THROW(register_self(), InitError, "ProvEventHandler register");
    ESP_LOGI(PEH_TAG, "init");
}

template <util::I_EventGroup EGroup, DftEventLoopHandle ELoop>
ProvEventHandler<EGroup, ELoop>::~ProvEventHandler() {
    ELoop::get().handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED,
                                    disconnect_handle);
    ELoop::get().handler_unregister(NETWORK_PROV_EVENT, ESP_EVENT_ANY_ID,
                                    handle);
    ProvEventHandler::event_group = nullptr;
    ESP_LOGI(PEH_TAG, "deinit");
}

template <util::I_EventGroup EGroup, DftEventLoopHandle ELoop>
esp_err_t ProvEventHandler<EGroup, ELoop>::register_self() {
    UTIL_RETURN(ELoop::get().handler_register(
        NETWORK_PROV_EVENT, ESP_EVENT_ANY_ID, &handler, NULL, &handle));
    UTIL_RETURN(ELoop::get().handler_register(
        WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, NULL,
        &disconnect_handle));
    return ESP_OK;
}

template <util::I_EventGroup EGroup, DftEventLoopHandle ELoop>
ProvReason ProvEventHandler<EGroup, ELoop>::get_reason() const noexcept {
    return reason;
}

template <util::I_EventGroup EGroup, DftEventLoopHandle ELoop>
void ProvEventHandler<EGroup, ELoop>::handler(void *arg,
                                              esp_event_base_t event_base,
                                              int32_t event_id,
                                              void *event_data) {
    (void)arg;
    (void)event_base;
    switch (event_id) {
    case (int32_t)ProvEvent::START:
        retries_left = retries;
        event_group->clear(wifi::end_bit | wifi::fail_bit | wifi::success_bit);
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
        event_group->set(wifi::success_bit);
        break;
    case (int32_t)ProvEvent::CRED_FAIL: {
        reason = *static_cast<ProvReason *>(event_data);
        ESP_LOGE(PEH_TAG,
                 "Provisioning failed!\n\tReason : %s"
                 "\n\tPlease reset to factory and retry provisioning",
                 (reason == ProvReason::AUTH_ERROR)
                     ? "Wi-Fi station authentication failed"
                     : "Wi-Fi access-point not found");
        event_group->set(wifi::fail_bit || wifi::end_bit);
        break;
    }
    case (int32_t)ProvEvent::END:
        ESP_LOGI(PEH_TAG, "Provisioning ended");
        event_group->set(wifi::end_bit);
        break;
    default:;
    }
}

template <util::I_EventGroup EGroup, DftEventLoopHandle ELoop>
void ProvEventHandler<EGroup, ELoop>::disconnect_handler(
    void *arg, esp_event_base_t event_base, int32_t event_id,
    void *event_data) {
    (void)arg;
    (void)event_base;
    (void)event_id;
    auto *data = static_cast<wifi_event_sta_disconnected_t *>(event_data);
    auto reason = static_cast<WiFiReason>(data->reason);
    if (wifi::is_disconnect_recoverable(reason) && --retries_left <= 0) {
        ESP_LOGI(PEH_TAG, "Too many retries. Failure");
        // wifi_prov_mgr's event handler will receive modified event_data.
        // The following reason will cause wifi_prov_mgr to stop retrying.
        data->reason = WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT;
    }
}
