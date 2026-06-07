#ifndef CREDENTIALS_WIFI_MANAGER_HPP
#include "CredentialsWiFiManager.hpp"
#endif

#include "global.hpp"

template <WiFiManagerTypes T>
template<typename Factory>
requires HandlerFactory<T, Factory>
    CredentialsWiFiManager<T>::CredentialsWiFiManager(std::string_view ssid,
                                                      std::string_view passwd,
                                                      Factory fctry)
    : ssid{ssid}, passwd{passwd}, event_group{}, handler{fctry(event_group)} {}

template <WiFiManagerTypes T> int CredentialsWiFiManager<T>::init() {
    UTIL_THROW(WiFi::get().set_mode(WiFiMode::STA), InitError,
               "Failed to set STA mode");
    auto cfg = wifi_config_t{};
    straux::strtoarr(cfg.sta.ssid, ssid, sizeof(cfg.sta.ssid));
    straux::strtoarr(cfg.sta.password, passwd, sizeof(cfg.sta.password));
    UTIL_THROW(WiFi::get().set_config(WiFiIF::STA, &cfg), InitError,
               "WiFi config fail");
    return ESP_OK;
}

template <WiFiManagerTypes T>
int CredentialsWiFiManager<T>::connect() {
    ESP_LOGI("CredentailsWiFiManager", "Connecting to %s", ssid.c_str());
    UTIL_THROW(WiFi::get().start(), WiFiManagerError, "Failed to start WiFi");
    UTIL_THROW(WiFi::get().set_ps(WiFiPSType::NONE), WiFiManagerError, "set ps");
    return wifi::await_connection(event_group);
}

template <WiFiManagerTypes T>
int CredentialsWiFiManager<T>::disconnect() {
    UTIL_RETURN(WiFi::get().stop());
    auto bits = event_group.wait(wifi::end_bit, pdFALSE, pdFALSE,
                                 10000 / portTICK_PERIOD_MS);
    return bits & wifi::end_bit ? ESP_OK : ESP_FAIL;
}

template <WiFiManagerTypes T>
bool CredentialsWiFiManager<T>::is_connected() {
    return event_group.get() & wifi::success_bit;
}
