#ifndef WIFIMANAGER_HPP
#include "WiFiManager.hpp"
#endif

#include "WiFiEventHandler.hpp"
#include "esp_log.h"
#include "global.hpp"
#include "wifi_aux.hpp"

template <WiFiManagerTypes T>
template <typename HandlerFactory>
    requires util::Returns<HandlerFactory, typename T::EventHandler,
                           typename T::EventGroup &>
WiFiManager<T>::WiFiManager(HandlerFactory handler_factory) noexcept
    : event_group{EGroup{}},
      handler{[this, ftry = std::move(handler_factory)]() {
          return ftry(event_group);
      }},
      provisioning{Provisioning{event_group, "ESP_PROV"}} {}

template <WiFiManagerTypes T> WiFiManager<T>::~WiFiManager() {
    WiFi::get().stop();
    event_group.wait(wifi::end_bit);
}

template <WiFiManagerTypes T> int WiFiManager<T>::init() {
    UTIL_THROW(WiFi::get().set_mode(WiFiMode::STA), InitError,
               "Failed to set STA mode");
    switch_to_prov();
    if (provisioning->is_provisioned()) {
        ESP_LOGI("WiFiManager", "ESP is provisioned");
        switch_to_no_prov();
    } else
        ESP_LOGI("WiFiManager", "ESP isn't provisioned");
    return ESP_OK;
}

template <WiFiManagerTypes T> int WiFiManager<T>::connect() {
    return provisioning.has_value() ? connect_with_prov()
                                    : connect_without_prov();
}

template <WiFiManagerTypes T> int WiFiManager<T>::disconnect() {
    UTIL_RETURN(WiFi::get().stop());
    auto bits = event_group.wait(wifi::end_bit, pdFALSE, pdFALSE,
                                 10000 / portTICK_PERIOD_MS);
    return bits & wifi::end_bit ? ESP_OK : ESP_FAIL;
}

template <WiFiManagerTypes T> bool WiFiManager<T>::is_connected() {
    return event_group.get() & wifi::success_bit;
}

template <WiFiManagerTypes T> int WiFiManager<T>::connect_with_prov() {
    auto status = provisioning->connect();
    if (status == ESP_OK)
        switch_to_no_prov();
    return status;
}

template <WiFiManagerTypes T> int WiFiManager<T>::connect_without_prov() {
    ESP_LOGI("WiFiManager", "Connecting...");
    UTIL_THROW(WiFi::get().start(), WiFiManagerError, "WiFi start");
    return wifi::await_connection(event_group);
}

template <WiFiManagerTypes T> void WiFiManager<T>::switch_to_prov() {
    provisioning.emplace(event_group, "ESP_PROV");
    handler.off();
}

template <WiFiManagerTypes T> void WiFiManager<T>::switch_to_no_prov() {
    handler.on();
    provisioning.reset();
}
