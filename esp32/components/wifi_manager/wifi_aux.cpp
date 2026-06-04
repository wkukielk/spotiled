#include "wifi_aux.hpp"
#include "esp_netif.h"
#include "global.hpp"
#include "util.hpp"

using namespace util;
using namespace std;

namespace wifi {
int await_connection(util::I_EventGroup &event_group) {
    auto bits = event_group.wait(success_bit | fail_bit);
    time().delay_ms(100); // let other event handlers run
    return bits & success_bit ? ESP_OK : ESP_FAIL;
}

const EventBits_t fail_bit = 1 << 0;
const EventBits_t success_bit = 1 << 1;
const EventBits_t end_bit = 1 << 2;
} // namespace wifi

esp_err_t mdns_resolve(const string &hostname, string &out, uint32_t timeout) {
    auto hostname_no_domain = hostname.ends_with(".local")
                                  ? hostname.substr(0, hostname.size() - 6)
                                  : hostname;
    esp_ip4_addr_t addr{};
    UTIL_RETURN(mdns_().query_a(hostname_no_domain.c_str(), timeout, &addr));

    out = straux::strprintf(IPSTR, IP2STR(&addr));
    return ESP_OK;
}

optional<URL> resolve_url(const URL &url, esp_err_t &err, uint32_t timeout) {
    auto hostname = url.get_hostname();
    if (!hostname.ends_with(".local")) {
        err = ESP_OK;
        return url;
    }

    auto ip_str = string{};
    if ((err = mdns_resolve(string{hostname}, ref(ip_str), timeout)))
        return std::nullopt;

    auto out = url;
    out.substitute_hostname(ip_str);
    return out;
}
