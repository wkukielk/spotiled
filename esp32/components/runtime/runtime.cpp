#include "runtime.hpp"
#include "global.hpp"
#include "util.hpp"
#include "wifi_aux.hpp"

// static const char *TAG = "Runtime";

std::unique_ptr<Runtime> Runtime::create(RuntimeConfig &&cfg) {
    UTIL_THROW(cfg.wifi_mgr->init(), InitError, "failed to init wifi mgr");
    UTIL_THROW(cfg.wifi_mgr->connect(), InitError, "Failed to connect to WiFi");

    auto otaurl = resolve_url(cfg.backend_cfg.base_url);
    auto otacfg = ota::ClientConfig{
        .httpcfg = {.url = otaurl.get(),
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

void Runtime::setup() { ota_client->update(); }
