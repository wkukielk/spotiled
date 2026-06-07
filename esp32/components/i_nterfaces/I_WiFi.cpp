#include "I_WiFi.hpp"
#include "util.hpp"

bool wifi::is_disconnect_recoverable(WiFiReason reason) {
    return reason == WiFiReason::AUTH_EXPIRE ||
           reason == WiFiReason::ASSOC_EXPIRE ||
           reason == WiFiReason::ASSOC_FAIL ||
           reason == WiFiReason::CONNECTION_FAIL ||
           reason == WiFiReason::ASSOC_COMEBACK_TIME_TOO_LONG;
}

#ifdef CONFIG_IDF_TARGET_LINUX
extern "C" {
ESP_EVENT_DEFINE_BASE(NETWORK_PROV_EVENT);
ESP_EVENT_DEFINE_BASE(WIFI_EVENT);
}
#endif
