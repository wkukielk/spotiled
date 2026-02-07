#include "CredentialsWiFiManager.hpp"
#include "global.hpp"
#include <cstring>

using util::end_bit;
using util::fail_bit;
using util::success_bit;

static const char *TAG = "WiFiManager";

CredentialsWiFiManager::CredentialsWiFiManager(
    std::string_view ssid, std::string_view passwd,
    u_ptr<util::EventHandler> handler, u_ptr<util::I_EventGroup> event_group)
    : ssid{ssid}, passwd{passwd}, handler{std::move(handler)},
      event_group{std::move(event_group)} {};

int CredentialsWiFiManager::init() {
    UTIL_THROW(wifi().set_mode(WiFiMode::STA), InitError,
               "Failed to set STA mode");
    auto cfg = wifi_config_t{};
    straux::strtoarr(cfg.sta.ssid, ssid, sizeof(cfg.sta.ssid));
    straux::strtoarr(cfg.sta.password, passwd, sizeof(cfg.sta.password));
    UTIL_THROW(wifi().set_config(WiFiIF::STA, &cfg), InitError,
               "WiFi config fail");
    return ESP_OK;
}

int CredentialsWiFiManager::connect() {
    UTIL_LOGI("Connecting to %s", ssid.c_str());
    UTIL_THROW(wifi().start(), WiFiManagerError, "Failed to start WiFi");
    UTIL_THROW(wifi().set_ps(WiFiPSType::NONE), WiFiManagerError, "set ps");
    return wifi::await_connection(*event_group);
}

int CredentialsWiFiManager::disconnect() {
    UTIL_RETURN(wifi().stop());
    auto bits = event_group->wait(end_bit, pdFALSE, pdFALSE,
                                  10000 / portTICK_PERIOD_MS);
    return bits & end_bit ? ESP_OK : ESP_FAIL;
}

bool CredentialsWiFiManager::is_connected() {
    return event_group->get() & success_bit;
}
