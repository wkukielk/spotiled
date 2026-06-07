#ifndef WIFI_EVENT_HANDLER_HPP
#include "WiFiEventHandler.hpp"
#endif

#include "errors.hpp"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "wifi_aux.hpp"

template <WiFiEventHandlerTypes T>
T::EventGroup *WiFiEventHandler<T>::event_group{};

template <WiFiEventHandlerTypes T>
uint8_t WiFiEventHandler<T>::max_retries{};

template <WiFiEventHandlerTypes T>
int8_t WiFiEventHandler<T>::retries_left{};

template <WiFiEventHandlerTypes T>
WiFiEventHandler<T>::WiFiEventHandler(EGroup &event_group,
                                                uint8_t retries) {
    UTIL_THROW(WiFiEventHandler::event_group != nullptr, InitError,
               "two handlers");
    UTIL_THROW(register_self(), InitError, "handler register");
    this->event_group = &event_group;
    this->max_retries = retries;
    this->retries_left = retries;
    event_group.set(wifi::end_bit);
}

template <WiFiEventHandlerTypes T>
WiFiEventHandler<T>::~WiFiEventHandler() {
    WiFiEventHandler::event_group = nullptr;
    DEL::get().handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, ip_instance);
    DEL::get().handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance);
}

template <WiFiEventHandlerTypes T>
esp_err_t WiFiEventHandler<T>::register_self() noexcept {
    UTIL_RETURN(DEL::get().handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                            &handler, NULL, &instance));
    UTIL_RETURN(DEL::get().handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                            &ip_handler, NULL, &ip_instance));
    return ESP_OK;
}

template <WiFiEventHandlerTypes T>
void WiFiEventHandler<T>::handler(void *arg,
                                            esp_event_base_t event_base,
                                            int32_t event_id,
                                            void *event_data) {
    (void)arg;
    (void)event_base;
    (void)event_id;
    switch (event_id) {
    case WIFI_EVENT_STA_START:
        event_group->clear(wifi::end_bit);
        retries_left = max_retries;
        WiFi::get().connect();
        break;
    case WIFI_EVENT_STA_DISCONNECTED: {
        event_group->clear(wifi::success_bit);
        auto *data = (wifi_event_sta_disconnected_t *)event_data;
        auto reason = static_cast<WiFiReason>(data->reason);
        ESP_LOGI("WiFiEventHandler", "Failed, reason: %hhu", (int)reason);
        if (wifi::is_disconnect_recoverable(reason) && --retries_left > 0) {
            ESP_LOGI("WiFiEventHandler", "Retrying...");
            WiFi::get().connect();
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

template <WiFiEventHandlerTypes T>
void WiFiEventHandler<T>::ip_handler(void *arg,
                                               esp_event_base_t event_base,
                                               int32_t event_id,
                                               void *event_data) {
    (void)arg;
    (void)event_base;
    (void)event_id;
    (void)event_data;
    event_group->clear(wifi::fail_bit);
    event_group->set(wifi::success_bit);
    retries_left = 0;
}
