#ifndef RUNTIME_HPP
#include "runtime.hpp"
#endif
#include "global.hpp"
#include "util.hpp"

template <typename EGroup>
template <typename T_FullOTA>
u_ptr<Runnable> Runtime<EGroup>::create(RuntimeConfig &&cfg) {
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
        .ota = make_unique<T_FullOTA>()};
    auto icfg = InnerConfig{.ota_client = OTAClient{move(otacfg)},
                            .wifi_mgr = move(cfg.wifi_mgr)};
    return u_ptr<Runnable>{new Runtime{move(icfg)}};
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
}

template <typename EGroup>
Runtime<EGroup>::Runtime(InnerConfig &&cfg)
    : wifi_mgr{move(cfg.wifi_mgr)}, egroup{},
      ota_thread{
          move(cfg.ota_client),
          util::JoinEventGroup{.egroup = this->egroup, .join_bit = OTA_BIT}},
      queue{4} {}

template <typename EGroup> void Runtime<EGroup>::run() {
    setup();
    int ret{};
    try {
        while ((ret = perform()) == ESP_OK)
            ;
    } catch (const std::exception &e) {
        ESP_LOGE(runtime_tag, "Exception: %s", e.what());
    }
    ESP_LOGE(runtime_tag, "Return: %d", ret);
    cleanup();
    ESP_LOGE(runtime_tag, "Runtime finished");
}

template <typename EGroup> void Runtime<EGroup>::setup() { ota_thread.start(); }

template <typename EGroup> int Runtime<EGroup>::perform() {
    auto data = pop_queue();
    int ret = data ? process(*data) : ESP_OK;
    return ret;
}

template <typename EGroup> void Runtime<EGroup>::cleanup() {
    ota_thread.join();
}

template <typename EGroup>
std::unique_ptr<master::Data> Runtime<EGroup>::pop_queue() {
    const auto delay_ms = 5000;
    auto data = queue.receive(delay_ms);
    if (data.get())
        ESP_LOGE(runtime_tag, "Queue: %s\n",
                 std::visit(master::DataPrinter{}, *data));
    return data;
}

template <typename EGroup>
esp_err_t Runtime<EGroup>::process(master::Data &data) {
    (void)data;
    return ESP_OK;
}
