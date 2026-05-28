#include "app.hpp"
#include "Configurer.hpp"
#include "WiFi.hpp"
#include "global.hpp"
#include "time_aux.hpp"
#include <memory>

using runtime_t = Runtime<util::EventGroup>;

u_ptr<Runnable> create_runtime(Config &&cfg) {
    return runtime_t::create<OTA>(RuntimeConfig{
        .backend_cfg = cfg.backend_cfg, .wifi_mgr = std::move(cfg.wifi_mgr)});
}

App::App(u_ptr<Runnable> &&runtime) : runtime{std::move(runtime)} {}

App *App::create() {
    auto cfgnvs = NVS{"cfg"};
    auto encnvs = NVS{"encnvs"};
    auto cfg_reader = Configurer{cfgnvs, encnvs};
    cfg_reader.preconfigure();
    Global::get_global()->set_nvs(mkuptr<NVS>());
    Global::get_global()->set_time(mkuptr<Time>());
    Global::get_global()->set_wifi(mkuptr<WiFi>(&wifi::cfg));
    auto cfg = cfg_reader.get_config();
    Global::get_global()->set_mdns(std::move(cfg.mdns));
    return new App{create_runtime(std::move(cfg))};
}

void App::run() { runtime->run(); }
