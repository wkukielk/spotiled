#include "wifi_aux.hpp"
#include "esp_netif.h"
#include "global.hpp"
#include "util.hpp"

using namespace util;
using namespace std;

const char *PEH_TAG = "ProvEventHandler";
const char *PROV_TAG = "Provisioning";

namespace provisioning {

const char *QRCODE_BASE_URL =
    "https://espressif.github.io/esp-jumpstart/qrcode.html";

uint8_t custom_service_uuid[16] = {
    /* little-endian */
    0xb4, 0xdf, 0x5a, 0x1c, 0x3f, 0x6b, 0xf4, 0xbf,
    0xea, 0x4a, 0x82, 0x03, 0x04, 0x90, 0x1a, 0x02,
};
} // namespace provisioning

namespace wifi {
int await_connection(util::EventGroupBase &event_group) {
    auto bits = event_group.wait(success_bit | fail_bit);
    time_().delay_ms(100); // let other event handlers run
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

namespace provisioning {
void print_qr(const char *name, const char *transport) {
    if (!name || !transport) {
        ESP_LOGW(PROV_TAG, "Cannot generate QR code payload. Data missing.");
        return;
    }
    char payload[150] = {0};
    snprintf(payload, sizeof(payload),
             "{\"ver\":\"%s\",\"name\":\"%s\""
             ",\"transport\":\"%s\"}",
             "v1", name, transport);
    ESP_LOGI(PROV_TAG, "Copy paste the below URL in a browser.\n%s?data=%s",
             QRCODE_BASE_URL, payload);
}

std::string get_service_name() {
    uint8_t eth_mac[6];
    const char *ssid_prefix = "PROV_";
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
    return straux::strprintf("%s%02X%02X%02X", ssid_prefix, eth_mac[3],
                             eth_mac[4], eth_mac[5]);
}
} // namespace provisioning
