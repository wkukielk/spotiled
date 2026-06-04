#ifndef WIFIMANAGER_HPP
#include "WiFiManager.hpp"
#endif

#include "WiFiEventHandler.hpp"
#include "esp_log.h"
#include "global.hpp"
#include "wifi_aux.hpp"

template <typename T>
template <typename HandlerFactory>
    requires Returns<HandlerFactory, typename T::EventHandler,
                     typename T::EventGroup &>
WiFiManager<T>::WiFiManager(
    HandlerFactory handler_factory,
    std::unique_ptr<I_Provisioning> &&provisioning) noexcept
    : event_group{EGroup{}},
      handler{[this, ftry = std::move(handler_factory)]() {
          return ftry(event_group);
      }},
      provisioning{std::move(provisioning)} {}

template <typename T> WiFiManager<T>::~WiFiManager() {
    wifi_().stop();
    event_group.wait(wifi::end_bit);
}

template <typename T> int WiFiManager<T>::init() {
    UTIL_THROW(wifi_().set_mode(WiFiMode::STA), InitError,
               "Failed to set STA mode");
    switch_to_prov();
    if (provisioning->is_provisioned()) {
        ESP_LOGI("WiFiManager", "ESP is provisioned");
        switch_to_no_prov();
    } else
        ESP_LOGI("WiFiManager", "ESP isn't provisioned");
    return ESP_OK;
}

template <typename T> int WiFiManager<T>::connect() {
    return provisioning->is_on() ? connect_with_prov() : connect_without_prov();
}

template <typename T> int WiFiManager<T>::disconnect() {
    UTIL_RETURN(wifi_().stop());
    auto bits = event_group.wait(wifi::end_bit, pdFALSE, pdFALSE,
                                 10000 / portTICK_PERIOD_MS);
    return bits & wifi::end_bit ? ESP_OK : ESP_FAIL;
}

template <typename T> bool WiFiManager<T>::is_connected() {
    return event_group.get() & wifi::success_bit;
}

template <typename T> int WiFiManager<T>::connect_with_prov() {
    auto status = provisioning->connect();
    if (status == ESP_OK)
        switch_to_no_prov();
    return status;
}

template <typename T> int WiFiManager<T>::connect_without_prov() {
    ESP_LOGI("WiFiManager", "Connecting...");
    UTIL_THROW(wifi_().start(), WiFiManagerError, "WiFi start");
    return wifi::await_connection(event_group);
}

template <typename T> void WiFiManager<T>::switch_to_prov() {
    provisioning->on();
    handler.off();
}

template <typename T> void WiFiManager<T>::switch_to_no_prov() {
    handler.on();
    provisioning->off();
}
