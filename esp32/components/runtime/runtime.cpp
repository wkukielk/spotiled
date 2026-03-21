#include "runtime.hpp"
#include "global.hpp"
#include "util.hpp"
#include "wifi_aux.hpp"

// static const char *TAG = "Runtime";

using namespace std;
using namespace util;

struct InnerConfig {
    u_ptr<OTAClient> ota_client;
    u_ptr<I_WiFiManager> wifi_mgr;
};

URL resolve_base(const URL &base_url) {
    auto base = optional<URL>{};
    retry(
        [&]() {
            auto err = esp_err_t{};
            base = resolve_url(base_url, err);
            return err;
        },
        5);
    UTIL_THROW(!base.has_value(), InitError, "Failed to resolve backend");
    return base.value();
}

u_ptr<Runtime> Runtime::create(RuntimeConfig &&cfg,
                               u_ptr<util::I_EventGroup> &&egroup) {
    UTIL_THROW(cfg.wifi_mgr->init(), InitError, "failed to init wifi mgr");
    UTIL_THROW(cfg.wifi_mgr->connect(), InitError, "Failed to connect to WiFi");

    auto base = resolve_base(cfg.backend_cfg.base_url);
    auto otaurl =
        base.get() + "/api/esp/firmware/" + CONFIG_IDF_TARGET "/latest";

    auto otacfg = ota::ClientConfig{
        .httpcfg = {.url = move(otaurl),
                    .auth_header = move(cfg.backend_cfg.auth_header),
                    .cert_pem = cfg.backend_cfg.cert_pem,
                    .cert_len = cfg.backend_cfg.cert_len,
                    .skip_cn_check = cfg.backend_cfg.skip_cn_check},
        .ota = move(cfg.ota)};
    auto icfg = InnerConfig{.ota_client = mkuptr<OTAClient>(move(otacfg)),
                            .wifi_mgr = move(cfg.wifi_mgr)};
    return u_ptr<Runtime>{new Runtime{move(icfg), move(egroup)}};
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
}

Runtime::Runtime(InnerConfig &&cfg, u_ptr<util::I_EventGroup> &&egroup)
    : wifi_mgr{move(cfg.wifi_mgr)}, ota_client{move(cfg.ota_client)},
      egroup{move(egroup)} {}

void Runtime::run() {
    setup();
    ota_thread->join();
}

void Runtime::setup() {
    auto t_params = util::thread_params{.fun = update_fun,
                                        .name = "ota",
                                        .fun_parameters = this,
                                        .join_group = egroup.get(),
                                        .join_bit = 1 << 0};
    ota_thread = mkuptr<util::Thread>(move(t_params));
}

void Runtime::update_fun(void *runtimePtr) {
    auto runtime = (Runtime *)runtimePtr;
    runtime->ota_client->update(); // TODO: reboot after unsuccessfull update
}
