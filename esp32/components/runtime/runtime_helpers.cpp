#include "runtime_helpers.hpp"
#include "wifi_aux.hpp"

using namespace std;

const char *runtime_tag = "Runtime";
const EventBits_t OTA_BIT = 1 << 0;

static void update_fun(void *ota_client_ptr);

OTAThread::OTAThread(OTAClient &&ota_client, const OTAEventBits &event_bits)
    : ota_client{move(ota_client)}, event_bits{event_bits} {}

void OTAThread::start() {
    thread.emplace(util::thread_params{.fun = update_fun,
                                       .name = "ota",
                                       .fun_parameters = &ota_client,
                                       .join_group = event_bits.egroup,
                                       .join_bit = event_bits.join_bit});
}

static void update_fun(void *ota_client_ptr) {
    auto ota_client = (OTAClient *)ota_client_ptr;
    ota_client->update(); // TODO: reboot after successfull update
}

URL resolve_base(const URL &base_url) {
    auto base = optional<URL>{};
    util::retry(
        [&]() {
            auto err = esp_err_t{};
            base = resolve_url(base_url, err);
            return err;
        },
        5);
    UTIL_THROW(!base.has_value(), InitError, "Failed to resolve backend");
    return base.value();
}
