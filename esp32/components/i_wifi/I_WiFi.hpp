#ifndef I_WIFI_HPP
#define I_WIFI_HPP

#include "esp_err.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "esp_event.h"
#pragma GCC diagnostic pop
#include "esp_wifi.h"

extern "C" {
ESP_EVENT_DECLARE_BASE(WIFI_EVENT);
ESP_EVENT_DECLARE_BASE(WIFI_PROV_EVENT);
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
    CONNECTION_FAIL = 205
};

class I_WiFi {
  public:
    // init(const wifi_init_config_t *config);
    virtual ~I_WiFi() noexcept {};
    virtual esp_err_t start() = 0;
    virtual esp_err_t stop() = 0;
    virtual esp_err_t set_config(WiFiIF ifx, wifi_config_t *conf) = 0;
    virtual esp_err_t set_mode(WiFiMode mode) = 0;
    virtual esp_err_t connect() = 0;
    virtual esp_err_t get_mac(WiFiIF ifx, uint8_t mac[6]) = 0;

    static bool is_disconnect_recoverable(WiFiReason reason);
};

#endif