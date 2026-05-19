#include "runtime.hpp"
#include "global.hpp"
#include "util.hpp"
#include "wifi_aux.hpp"

static const char *TAG = "Runtime";

using namespace std;
using namespace util;
using namespace runtime;

static const EventBits_t OTA_JOIN_BIT = 1 << 0;

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

StateFactory::StateFactory() {}

std::unique_ptr<sm::State<StateData, Data>>
StateFactory::get(runtime::StateData &&data) const {
    auto state = std::visit(
        overloaded{
            [](IdleStateData &&) { return mkuptr<IdleState>(); },
            [](ReconnectStateData &&) { return mkuptr<IdleState>(); }, // TODO
            [](std::monostate &&) { return u_ptr<IdleState>{nullptr}; }},
        std::move(data));
    UTIL_THROW(!state, NullError, "state factory got monostate");
    return state;
}

u_ptr<Runtime> Runtime::create(RuntimeConfig &&cfg,
                               u_ptr<I_EventGroup> &&egroup) {
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

Runtime::Runtime(InnerConfig &&cfg, u_ptr<I_EventGroup> &&egroup)
    : wifi_mgr{move(cfg.wifi_mgr)}, ota_client{move(cfg.ota_client)},
      egroup{move(egroup)}, queue{4}, state{make_unique<IdleState>()},
      state_factory{StateFactory{}} {}

void Runtime::run() {
    setup();
    int ret{};
    try {
        while ((ret = loop()) == ESP_OK)
            ;
    } catch (const std::exception &e) {
        UTIL_LOGE("Exception: %s", e.what());
    }
    UTIL_LOGE("Return: %d", ret);
    cleanup();
    UTIL_LOGI("Runtime finished");
}

void Runtime::notify(u_ptr<Data> &&data) { queue.send(move(data)); }

void Runtime::setup() {
    auto ota_thread_params = thread_params{.fun = update_fun,
                                           .name = "ota",
                                           .fun_parameters = this,
                                           .join_group = egroup.get(),
                                           .join_bit = OTA_JOIN_BIT};
    ota_thread = mkuptr<Thread>(move(ota_thread_params));
}

int Runtime::loop() {
    auto data = pop_queue();
    int ret = data ? state->perform(*data) : ESP_OK;
    auto next_state_data = state->get_next_state_data();
    if (!holds_alternative<monostate>(next_state_data))
        change_state(move(next_state_data));
    return ret;
}

void Runtime::cleanup() { ota_thread->join(); }

std::unique_ptr<Data> Runtime::pop_queue() {
    auto data = queue.receive_nowait();
    if (data.get())
        UTIL_LOGI("Queue: %s\n", std::visit(DataPrinter{}, *data));
    return data;
}

void Runtime::change_state(StateData &&next_state_data) {
    UTIL_LOGI("New state: %s\n",
              std::visit(StateDataPrinter{}, next_state_data));
    state = state_factory.get(std::move(next_state_data));
}

void Runtime::update_fun(void *runtimePtr) {
    auto runtime = (Runtime *)runtimePtr;
    runtime->ota_client->update(); // TODO: reboot after unsuccessfull update
}
