#ifndef RUNTIME_HPP
#include "runtime.hpp"
#endif
#include "global.hpp"
#include "util.hpp"

template <typename T> u_ptr<Runtime<T>> Runtime<T>::create(RuntimeConfig &&cfg) {
    using T_OTAClient = typename T_OTAThread::Client;

    cfg.wifi_mgr->init();
    UTIL_THROW(cfg.wifi_mgr->connect(), InitError, "Failed to connect to WiFi");

    auto base = resolve_base(cfg.backend_cfg.base_url);
    auto otaurl =
        base.get() + "/api/esp/firmware/" + CONFIG_IDF_TARGET "/latest";

    auto otacfg =
        ota::HTTPSConfig{.url = move(otaurl),
                         .auth_header = move(cfg.backend_cfg.auth_header),
                         .cert_pem = cfg.backend_cfg.cert_pem,
                         .cert_len = cfg.backend_cfg.cert_len,
                         .skip_cn_check = cfg.backend_cfg.skip_cn_check};
    auto icfg =
        InnerConfig<T_OTAClient>{.ota_client = T_OTAClient{move(otacfg)},
                                 .wifi_mgr = move(cfg.wifi_mgr)};
    return u_ptr<Runtime<T>>(new Runtime<T>(move(icfg)));
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
}

template <typename EGroup>
util::JoinEventGroup get_ota_join_group(EGroup &egroup) {
    return util::JoinEventGroup{.egroup = egroup, .join_bit = OTA_BIT};
}

template <typename EGroup>
util::JoinEventGroup get_server_join_group(EGroup &egroup) {
    return util::JoinEventGroup{.egroup = egroup, .join_bit = SERVER_BIT};
}

template <typename T>
Runtime<T>::Runtime(InnerConfig<T_OTAClient> &&cfg)
    : wifi_mgr{move(cfg.wifi_mgr)}, egroup{},
      ota_thread{move(cfg.ota_client), get_ota_join_group(egroup)},
      server_thread{get_server_join_group(egroup)}, queue{4} {}

template <typename T> void Runtime<T>::run() {
    int ret = setup();
    if (ret == ESP_OK) {
        ESP_LOGI(runtime_tag, "%s", "Setup ok");
        try {
            while ((ret = perform()) == ESP_OK)
                ;
        } catch (const std::exception &e) {
            ESP_LOGE(runtime_tag, "Exception: %s", e.what());
        }
    }
    ESP_LOGE(runtime_tag, "Return: %d", ret);
    cleanup();
    ESP_LOGE(runtime_tag, "Runtime finished");
}

template <typename T> esp_err_t Runtime<T>::setup() {
    ota_thread.start();
    return server_thread.start();
}

template <typename T> int Runtime<T>::perform() {
    auto data = pop_queue();
    int ret = data ? process(*data) : ESP_OK;
    return ret;
}

template <typename T> void Runtime<T>::cleanup() {
    ota_thread.join();
    server_thread.join();
}

template <typename T> std::unique_ptr<master::Data> Runtime<T>::pop_queue() {
    const auto delay_ms = 5000;
    auto data = queue.receive(delay_ms);
    if (data.get())
        ESP_LOGE(runtime_tag, "Queue: %s\n",
                 std::visit(master::DataPrinter{}, *data));
    return data;
}

template <typename T> esp_err_t Runtime<T>::process(master::Data &data) {
    (void)data;
    return ESP_OK;
}
