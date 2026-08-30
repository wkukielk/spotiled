#ifndef I_WIFI_HPP
#define I_WIFI_HPP

#include "esp_err.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "esp_event.h"
#pragma GCC diagnostic pop
#include "esp_wifi.h"
#include <concepts>

extern "C" {
ESP_EVENT_DECLARE_BASE(WIFI_EVENT);
ESP_EVENT_DECLARE_BASE(NETWORK_PROV_EVENT);
}

enum class WiFiMode {
    MODE_NULL = 0, /**< Null mode */
    STA,           /**< Wi-Fi station mode */
    AP,            /**< Wi-Fi soft-AP mode */
    APSTA,         /**< Wi-Fi station + soft-AP mode */
};

enum class WiFiIF { STA = 0, AP };

enum class WiFiReason {
    AUTH_EXPIRE = 2,
    ASSOC_EXPIRE = 4,
    ASSOC_FAIL = 203,
    CONNECTION_FAIL = 205,
    ASSOC_COMEBACK_TIME_TOO_LONG = 208
};

enum class WiFiPSType {
    NONE,
    MIN_MODEM,
    MAX_MODEM,
};

template <typename T>
concept I_WiFi = requires(T t, WiFiIF ifx, wifi_config_t *conf, WiFiMode mode,
                          WiFiPSType ps, uint8_t mac[6]) {
    { t.start() } noexcept -> std::same_as<esp_err_t>;
    { t.stop() } noexcept -> std::same_as<esp_err_t>;
    { t.disconnect() } noexcept -> std::same_as<esp_err_t>;
    { t.set_config(ifx, conf) } noexcept -> std::same_as<esp_err_t>;
    { t.set_mode(mode) } noexcept -> std::same_as<esp_err_t>;
    { t.set_ps(ps) } noexcept -> std::same_as<esp_err_t>;
    { t.connect() } noexcept -> std::same_as<esp_err_t>;
    { t.get_mac(ifx, mac) } noexcept -> std::same_as<esp_err_t>;
};

template <typename T>
concept WiFiHandle = std::constructible_from<T> && requires {
    { T::get() } -> I_WiFi;
};

namespace wifi {
bool is_disconnect_recoverable(WiFiReason reason);
}

#endif
