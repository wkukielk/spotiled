#include "runtime.hpp"
#include "global.hpp"
#include "util.hpp"
#include "wifi_aux.hpp"

// static const char *TAG = "Runtime";

using namespace std;

struct InnerConfig {
    u_ptr<OTAClient> ota_client;
    u_ptr<I_WiFiManager> wifi_mgr;
};

u_ptr<Runtime> Runtime::create(RuntimeConfig &&cfg) {
    UTIL_THROW(cfg.wifi_mgr->init(), InitError, "failed to init wifi mgr");
    UTIL_THROW(cfg.wifi_mgr->connect(), InitError, "Failed to connect to WiFi");

    auto base = resolve_url(cfg.backend_cfg.base_url);
    auto otaurl = base.get() + "/api/firmware/" + CONFIG_IDF_TARGET "/latest";

    auto otacfg = ota::ClientConfig{
        .httpcfg = {.url = move(otaurl),
                    .cert_pem = cfg.backend_cfg.cert_pem,
                    .cert_len = cfg.backend_cfg.cert_len,
                    .skip_cn_check = cfg.backend_cfg.skip_cn_check},
        .ota = move(cfg.ota)};
    auto icfg = InnerConfig{.ota_client = mkuptr<OTAClient>(move(otacfg)),
                            .wifi_mgr = move(cfg.wifi_mgr)};
    return u_ptr<Runtime>{new Runtime{move(icfg)}};
}

Runtime::Runtime(InnerConfig &&cfg)
    : wifi_mgr{move(cfg.wifi_mgr)}, ota_client{move(cfg.ota_client)} {}

void Runtime::run() { setup(); }

void Runtime::setup() { ota_client->update(); }
