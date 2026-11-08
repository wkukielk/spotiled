#ifndef RUNTIME_HPP
#include "runtime.hpp"
#endif
#include "global.hpp"
#include "util.hpp"

template <typename T>
u_ptr<Runtime<T>> Runtime<T>::create(RuntimeConfig &&cfg) {
    cfg.wifi_mgr->init();
    return u_ptr<Runtime<T>>(new Runtime<T>(std::move(cfg)));
}

template <typename T>
auto Runtime<T>::make_ota_client() -> std::expected<T_OTAClient, esp_err_t> {
    auto base = resolve_base(backend_cfg.base_url);
    if (!base)
        return std::unexpected(base.error());
    auto otaurl =
        base->get() + "/api/esp/firmware/" + CONFIG_IDF_TARGET "/latest";

    auto otacfg = ota::HTTPSConfig{.url = move(otaurl),
                                   .auth_header = move(backend_cfg.auth_header),
                                   .cert_pem = backend_cfg.cert_pem,
                                   .cert_len = backend_cfg.cert_len,
                                   .skip_cn_check = backend_cfg.skip_cn_check};
    return T_OTAClient{std::move(otacfg)};
}

template <typename EGroup>
util::JoinEventGroup get_ota_join_group(EGroup &egroup) {
    return util::JoinEventGroup{.egroup = egroup, .join_bit = OTA_BIT};
}

template <typename EGroup>
util::JoinEventGroup get_server_join_group(EGroup &egroup) {
    return util::JoinEventGroup{.egroup = egroup, .join_bit = SERVER_BIT};
}

template <typename EGroup>
util::JoinEventGroup get_udp_join_group(EGroup &egroup) {
    return util::JoinEventGroup{.egroup = egroup, .join_bit = UDP_BIT};
}

template <typename T>
Runtime<T>::Runtime(RuntimeConfig &&cfg)
    : backend_cfg{std::move(cfg.backend_cfg)},
      wifi_mgr{std::move(cfg.wifi_mgr)}, status_light{status_light_pin},
      egroup{}, ledc{}, server_thread{mkuptr<T_ServerHandler>(ledc, *this),
                                      get_server_join_group(egroup)},
      udp_thread{mkuptr<T_UDPHandler>(ledc), get_udp_join_group(egroup)},
      queue{4} {
    status_light.on();
    wifi_mgr->set_on_disconnected([this] {
        queue.send_nowait(mkuptr<master::Data>(
            master::ConnectionErrorData{runtime::TaskID::MASTER}));
    });
}

template <typename T> void Runtime<T>::run() {
    notify(
        mkuptr<master::Data>(master::ConnectRequest{runtime::TaskID::MASTER}));
    int ret = ESP_OK;
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

// Runs a step, turning exceptions into ESP_FAIL and logging failures.
template <typename T>
template <typename F>
esp_err_t Runtime<T>::guarded(const char *step, F &&f) noexcept {
    auto err = esp_err_t{ESP_FAIL};
    try {
        err = f();
    } catch (const std::exception &e) {
        ESP_LOGE(runtime_tag, "%s exception: %s", step, e.what());
    }
    if (err == ESP_OK)
        ESP_LOGI(runtime_tag, "%s ok", step);
    else
        ESP_LOGE(runtime_tag, "%s failed: %d", step, err);
    return err;
}

// On success queues StartRequest; on failure signals a WiFi error.
template <typename T> void Runtime<T>::handle_connect() noexcept {
    if (guarded("Connect", [this] { return connect(); }) != ESP_OK) {
        status_light.blink(wifi_error_blink);
        return;
    }
    // Sent from the queue's own consumer, so it must not block.
    auto status = queue.send_nowait(
        mkuptr<master::Data>(master::StartRequest{runtime::TaskID::MASTER}));
    if (status != util::SendStatus::success) {
        ESP_LOGE(runtime_tag, "%s", "Queue full, can't start");
        status_light.blink(error_blink);
    }
}

template <typename T> esp_err_t Runtime<T>::connect() {
    if (wifi_mgr->will_provision())
        status_light.blink(provisioning_blink);
    UTIL_RETURN(wifi_mgr->connect());
    status_light.on();
    return ESP_OK;
}

template <typename T> void Runtime<T>::handle_start() noexcept {
    if (guarded("Start", [this] { return start(); }) != ESP_OK)
        status_light.blink(error_blink);
}

template <typename T> esp_err_t Runtime<T>::start() {
    auto ota_client = make_ota_client();
    if (!ota_client)
        return ota_client.error();
    ota_thread.emplace(std::move(*ota_client), get_ota_join_group(egroup));
    ota_thread->start();
    UTIL_RETURN(server_thread.start());
    return udp_thread.start();
}

template <typename T> int Runtime<T>::perform() {
    auto data = pop_queue();
    int ret = data ? process(*data) : ESP_OK;
    return ret;
}

template <typename T> void Runtime<T>::cleanup() {
    if (ota_thread)
        ota_thread->join();
    server_thread.join();
    udp_thread.join();
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
    if (std::holds_alternative<master::ConnectRequest>(data))
        handle_connect();
    else if (std::holds_alternative<master::StartRequest>(data))
        handle_start();
    else if (std::holds_alternative<master::ConnectionErrorData>(data))
        status_light.blink(wifi_error_blink);
    else if (std::holds_alternative<master::ReprovisionRequest>(data))
        wifi_mgr->reprovision();
    return ESP_OK;
}
