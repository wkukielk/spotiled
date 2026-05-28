#include "master/runtime_helpers.hpp"
#include "wifi_aux.hpp"

using namespace std;
using namespace util;

const char *runtime_tag = "Runtime";
const char *server_tag = "Server";
const EventBits_t OTA_BIT = 1 << 0;

static void ota_thread_fun(void *ota_client_ptr);

OTAThread::OTAThread(OTAClient &&ota_client, const JoinEventGroup &join_group)
    : PostponedThread{join_group}, ota_client{move(ota_client)} {}

void OTAThread::start() {
    thread.emplace(util::thread_params{.fun = ota_thread_fun,
                                       .name = "ota",
                                       .fun_parameters = &ota_client,
                                       .join_group = join_group.egroup,
                                       .join_bit = join_group.join_bit});
}

static void ota_thread_fun(void *ota_client_ptr) {
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
