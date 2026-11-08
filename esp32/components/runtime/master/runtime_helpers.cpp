#include "master/runtime_helpers.hpp"
#include "global.hpp"
#include "wifi_aux.hpp"
#include <cstring>

using namespace std;
using namespace util;

const char *runtime_tag = "Runtime";
const char *server_tag = "Server";
const EventBits_t OTA_BIT = 1 << 0;
const EventBits_t SERVER_BIT = 1 << 1;
const EventBits_t UDP_BIT = 1 << 2;

expected<URL, esp_err_t> resolve_base(const URL &base_url) {
    auto base = optional<URL>{};
    auto err = util::retry(
        [&]() {
            auto err = esp_err_t{};
            base = resolve_url(base_url, err);
            ESP_LOGI("resolve_base", "ret: %d", err);
            return err;
        },
        15);
    if (!base.has_value()) {
        ESP_LOGE("resolve_base", "Failed to resolve backend");
        return unexpected(err != ESP_OK ? err : ESP_FAIL);
    }
    return base.value();
}

namespace force_update {
namespace {
const char *key = "force_update";
const char *set = "1";
const char *unset = "0";
} // namespace

void schedule() { nvs_().write(key, set); }

bool is_scheduled() noexcept {
    try {
        return strcmp(nvs_().read(key, unset).c_str(), set) == 0;
    } catch (const exception &e) {
        ESP_LOGE(runtime_tag, "Read force update flag: %s", e.what());
        return false;
    }
}

void clear() noexcept {
    try {
        nvs_().write(key, unset);
    } catch (const exception &e) {
        ESP_LOGE(runtime_tag, "Clear force update flag: %s", e.what());
    }
}
} // namespace force_update
