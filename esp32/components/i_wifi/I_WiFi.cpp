#include "I_WiFi.hpp"
#include "util.hpp"

bool I_WiFi::is_disconnect_recoverable(WiFiReason reason) {
    return reason == WiFiReason::AUTH_EXPIRE ||
           reason == WiFiReason::ASSOC_EXPIRE ||
           reason == WiFiReason::ASSOC_FAIL ||
           reason == WiFiReason::CONNECTION_FAIL;
}

#ifdef CONFIG_IDF_TARGET_LINUX
extern "C" {
ESP_EVENT_DEFINE_BASE(WIFI_PROV_EVENT);
ESP_EVENT_DEFINE_BASE(WIFI_EVENT);
}
#endif
