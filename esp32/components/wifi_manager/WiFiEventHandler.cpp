#include "WiFiEventHandler.hpp"
#include "WiFi.hpp"
#include "errors.hpp"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "global.hpp"

static const char *TAG = "WiFiEventHandler";
util::I_EventGroup *WiFiEventHandler::event_group{};
uint8_t WiFiEventHandler::max_retries{};
int8_t WiFiEventHandler::retries_left{};
esp_event_handler_instance_t WiFiEventHandler::handle{};
esp_event_handler_instance_t WiFiEventHandler::ip_handle{};
const util::cbdata WiFiEventHandler::cbdata[2]{
    {.callback = handler, .base = WIFI_EVENT, .id = ESP_EVENT_ANY_ID},
    {.callback = ip_handler, .base = IP_EVENT, .id = IP_EVENT_STA_GOT_IP}};

WiFiEventHandler::WiFiEventHandler(util::I_EventGroup &event_group,
                                   uint8_t retries, util::Register reg) {
    UTIL_THROW(WiFiEventHandler::event_group != nullptr, InitError,
               "two handlers");
    this->event_group = &event_group;
    this->max_retries = retries;
    this->retries_left = retries;
    event_group.set(util::end_bit);
    if (reg == util::Register::YES)
        UTIL_THROW(register_self(), InitError, "handler register");
}

WiFiEventHandler::~WiFiEventHandler() {
    esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                          ip_handle);
    esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, handle);
    WiFiEventHandler::event_group = nullptr;
}

esp_err_t WiFiEventHandler::register_self() {
    UTIL_RETURN(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &handler, NULL, &handle));
    UTIL_RETURN(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_handler, NULL, &ip_handle));
    return ESP_OK;
}

std::span<const util::cbdata> WiFiEventHandler::get_cbdata() const noexcept {
    return straux::to_span(cbdata, 2);
}

void WiFiEventHandler::handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
    switch (event_id) {
    case WIFI_EVENT_STA_START:
        event_group->clear(util::end_bit);
        retries_left = max_retries;
        wifi().connect();
        break;
    case WIFI_EVENT_STA_CONNECTED: {
        auto netif = ((WiFi &)wifi()).get_netif();
        esp_netif_dhcpc_stop(netif);
        esp_netif_ip_info_t ip_info{};
        esp_netif_str_to_ip4("192.168.50.198", &ip_info.ip);
        esp_netif_str_to_ip4("192.168.50.1", &ip_info.gw);
        esp_netif_str_to_ip4("255.255.255.0", &ip_info.netmask);

        esp_netif_set_ip_info(netif, &ip_info);
        break;
    }
    case WIFI_EVENT_STA_DISCONNECTED: {
        event_group->clear(util::success_bit);
        auto *data = (wifi_event_sta_disconnected_t *)event_data;
        auto reason = static_cast<WiFiReason>(data->reason);
        UTIL_LOGW("Failed, reason: %hhu", (int)reason);
        if (I_WiFi::is_disconnect_recoverable(reason) && --retries_left > 0) {
            UTIL_LOGW("Retrying...");
            wifi().connect();
        } else
            event_group->set(util::fail_bit);
        break;
    }
    case WIFI_EVENT_STA_STOP:
        event_group->clear(util::success_bit | util::fail_bit);
        event_group->set(util::end_bit);
        break;
    default:;
    }
}

void WiFiEventHandler::ip_handler(void *arg, esp_event_base_t event_base,
                                  int32_t event_id, void *event_data) {
    event_group->clear(util::fail_bit);
    event_group->set(util::success_bit);
    retries_left = 0;
}
