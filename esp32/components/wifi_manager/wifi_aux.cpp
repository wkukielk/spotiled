#include "wifi_aux.hpp"
#include "esp_netif.h"
#include "global.hpp"
#include "util.hpp"

using util::end_bit;
using util::fail_bit;
using util::success_bit;

namespace wifi {
int await_connection(util::I_EventGroup &event_group) {
    auto bits = event_group.wait(success_bit | fail_bit);
    return bits & success_bit ? ESP_OK : ESP_FAIL;
}
} // namespace wifi

std::string mdns_resolve(const std::string &hostname) {
    auto hostname_no_domain = hostname.ends_with(".local")
                                  ? hostname.substr(0, hostname.size() - 6)
                                  : hostname;
    esp_ip4_addr_t addr{};
    auto err = mdns().query_a(hostname_no_domain.c_str(), 1000, &addr);
    UTIL_THROW(err, InitError, "failed to resolve ota hostname");
    return straux::strprintf(IPSTR, IP2STR(&addr));
}
