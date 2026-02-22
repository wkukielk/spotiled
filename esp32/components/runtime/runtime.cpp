#include "runtime.hpp"
#include "global.hpp"
#include "util.hpp"
#include "wifi_aux.hpp"

// static const char *TAG = "Runtime";

std::string get_ota_url(const URL &ota_base_url) {
    auto hostname = ota_base_url.get_hostname();
    if (!hostname.ends_with(".local"))
        return ota_base_url.get();

    auto iphostname = mdns_resolve(std::string{hostname});
    auto ota_url = ota_base_url;
    ota_url.substitute_hostname(iphostname);
    return ota_url.get();
}

std::unique_ptr<Runtime> Runtime::create(RuntimeConfig &&cfg) {
    UTIL_THROW(cfg.wifi_mgr->init(), InitError, "failed to init wifi mgr");
    UTIL_THROW(cfg.wifi_mgr->connect(), InitError, "Failed to connect to WiFi");

    auto otaurl = get_ota_url(cfg.backend_cfg.base_url);
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

void Runtime::setup() {}
