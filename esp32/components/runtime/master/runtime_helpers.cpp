#include "master/runtime_helpers.hpp"
#include "wifi_aux.hpp"

using namespace std;
using namespace util;

const char *runtime_tag = "Runtime";
const char *server_tag = "Server";
const EventBits_t OTA_BIT = 1 << 0;
const EventBits_t SERVER_BIT = 1 << 1;
const EventBits_t UDP_BIT = 1 << 2;

URL resolve_base(const URL &base_url) {
    auto base = optional<URL>{};
    util::retry(
        [&]() {
            auto err = esp_err_t{};
            base = resolve_url(base_url, err);
            ESP_LOGI("resolve_base", "ret: %d", err);
            return err;
        },
        15);
    UTIL_THROW(!base.has_value(), InitError, "Failed to resolve backend");
    return base.value();
}
