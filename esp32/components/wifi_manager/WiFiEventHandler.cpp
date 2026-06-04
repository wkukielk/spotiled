#include "WiFiEventHandler.hpp"
#include "errors.hpp"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "global.hpp"
#include "wifi_aux.hpp"
#include <algorithm>
#include <iterator>

static const char *TAG = "WiFiEventHandler";
util::I_EventGroup *WiFiEventHandler::event_group{};
uint8_t WiFiEventHandler::max_retries{};
int8_t WiFiEventHandler::retries_left{};

std::vector<util::cbdata> WiFiEventHandler::cbdata{
    {.callback = handler, .base = WIFI_EVENT, .id = ESP_EVENT_ANY_ID},
    {.callback = ip_handler, .base = IP_EVENT, .id = IP_EVENT_STA_GOT_IP}};

WiFiEventHandler::WiFiEventHandler(util::I_EventGroup &event_group,
                                   uint8_t retries, util::Register reg) {
    UTIL_THROW(WiFiEventHandler::event_group != nullptr, InitError,
               "two handlers");
    this->event_group = &event_group;
    this->max_retries = retries;
    this->retries_left = retries;
    event_group.set(wifi::end_bit);
    if (reg == util::Register::YES)
        UTIL_THROW(register_self(), InitError, "handler register");
}

WiFiEventHandler::~WiFiEventHandler() {
    esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                          cbdata[0].instance);
    esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                          cbdata[1].instance);
    WiFiEventHandler::event_group = nullptr;
    for (auto &cb : cbdata) {
        cb.handler = nullptr;
        cb.instance = nullptr;
    };
}

esp_err_t WiFiEventHandler::register_self() {
    UTIL_RETURN(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &handler, NULL, &cbdata[0].instance));
    UTIL_RETURN(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_handler, NULL, &cbdata[1].instance));
    std::ranges::for_each(cbdata, [this](auto &cb) { cb.handler = this; });
    return ESP_OK;
}

std::span<const util::cbdata> WiFiEventHandler::get_cbdata() const noexcept {
    return cbdata;
}

void WiFiEventHandler::handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
    switch (event_id) {
    case WIFI_EVENT_STA_START:
        event_group->clear(wifi::end_bit);
        retries_left = max_retries;
        wifi_().connect();
        break;
    case WIFI_EVENT_STA_DISCONNECTED: {
        event_group->clear(wifi::success_bit);
        auto *data = (wifi_event_sta_disconnected_t *)event_data;
        auto reason = static_cast<WiFiReason>(data->reason);
        UTIL_LOGW("Failed, reason: %hhu", (int)reason);
        if (I_WiFi::is_disconnect_recoverable(reason) && --retries_left > 0) {
            UTIL_LOGW("Retrying...");
            wifi_().connect();
        } else
            event_group->set(wifi::fail_bit);
        break;
    }
    case WIFI_EVENT_STA_STOP:
        event_group->clear(wifi::success_bit | wifi::fail_bit);
        event_group->set(wifi::end_bit);
        break;
    default:;
    }
}

void WiFiEventHandler::ip_handler(void *arg, esp_event_base_t event_base,
                                  int32_t event_id, void *event_data) {
    event_group->clear(wifi::fail_bit);
    event_group->set(wifi::success_bit);
    retries_left = 0;
}
