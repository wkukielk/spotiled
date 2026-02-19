#include "runtime.hpp"
#include "global.hpp"

static const char *TAG = "Runtime";

std::string get_ota_url(const URL &ota_base_url) {
    auto hostname = ota_base_url.get_hostname();
    if (!hostname.ends_with(".local"))
        return ota_base_url.get();

    esp_ip4_addr_t addr;
    mdns().query_a(std::string{hostname}.c_str(), 1000, &addr);
    hostname = straux::strprintf(IPSTR, IP2STR(&addr));
    auto ota_url = ota_base_url;
    ota_url.substitute_hostname(hostname);
    return ota_url.get();
}

std::unique_ptr<Runtime> Runtime::create(RuntimeConfig &&cfg) {
    UTIL_THROW(cfg.wifi_mgr->init(), InitError, "failed to init wifi mgr");
    UTIL_THROW(cfg.wifi_mgr->connect(), InitError, "Failed to connect to WiFi");
    auto otaurl = get_ota_url(std::move(cfg.backend_cfg.base_url));
    auto otacfg = ota::ClientConfig{
        .httpcfg = {.url = std::move(otaurl),
                    .cert_pem = cfg.backend_cfg.cert_pem,
                    .cert_len = cfg.backend_cfg.cert_len,
                    .skip_cn_check = cfg.backend_cfg.skip_cn_check},
        .ota = std::move(cfg.ota)};
    auto icfg =
        InnerRuntimeConfig{.ota_client = mkuptr<OTAClient>(std::move(otacfg)),
                           .wifi_mgr = std::move(cfg.wifi_mgr)};
    return u_ptr<Runtime>{new Runtime{std::move(icfg)}};
}

Runtime::Runtime(InnerRuntimeConfig &&cfg)
    : wifi_mgr{std::move(cfg.wifi_mgr)}, ota_client{std::move(cfg.ota_client)} {
}

void Runtime::run() { setup(); }

void Runtime::setup() {
    UTIL_THROW(wifi_mgr->init(), InitError, "failed to init wifi mgr");
    UTIL_THROW(wifi_mgr->connect(), InitError, "Failed to connect to WiFi");
    ESP_LOGI(TAG, "Connected to WiFi");
}
