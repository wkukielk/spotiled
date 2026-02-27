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
    auto hname = hostname_no_domain.c_str();
    auto err = esp_err_t{};
    err = util::retry([&]() { return mdns().query_a(hname, 1000, &addr); }, 5);
    UTIL_THROW(err, InitError, "failed to resolve ota hostname");
    return straux::strprintf(IPSTR, IP2STR(&addr));
}

URL resolve_url(const URL &url) {
    auto hostname = url.get_hostname();
    if (!hostname.ends_with(".local"))
        return url;

    auto ip_str = mdns_resolve(std::string{hostname});
    auto url_out = url;
    url_out.substitute_hostname(ip_str);
    return url_out;
}
