#include "app.hpp"
#include "Configurer.hpp"
#include "NVSSecurity.hpp"
#include "WiFi.hpp"
#include "global.hpp"
#include "server/server.hpp"
#include "time_aux.hpp"
#include <memory>

using namespace std;

App::App(AppHandles &&handles) : handles{move(handles)} {}

struct NVSHandles {
    u_ptr<NVSSecurity> nvs_security;
    NVS cfg;
    NVS enc;
    NVSHandle dft;
};

NVSHandles init_nvs() {
    auto nvs_security = mkuptr<NVSSecurity>();
    auto cfgnvs = NVS{"cfg", *nvs_security};
    auto encnvs = NVS{"encnvs", *nvs_security};
    auto nvs = mkuptr<NVS>(*nvs_security);
    nvs->init();
    cfgnvs.init();
    encnvs.init();

    return {.nvs_security{move(nvs_security)},
            .cfg{move(cfgnvs)},
            .enc{move(encnvs)},
            .dft{move(nvs)}};
}

App App::create() {
    auto nvs_handles = init_nvs();

    auto cfg_reader = Configurer{nvs_handles.cfg, nvs_(), nvs_handles.enc};
    cfg_reader.preconfigure();

    auto eloop = Handle<DftEventLoop>{};
    auto netif = Netif{};
    auto time = TimeHandle{mkuptr<Time>()};
    auto led = Handle<LED>{};

    auto cfg = cfg_reader.get_config();
    auto mdns = MDNSHandle{move(cfg.mdns)};

    auto runtime = runtime_t::create(RuntimeConfig{
        .backend_cfg = cfg.backend_cfg, .wifi_mgr = move(cfg.wifi_mgr)});

    auto handles = AppHandles{
        .eloop{move(eloop)},
        .netif{move(netif)},
        .nvs_security{move(nvs_handles.nvs_security)},
        .nvs{move(nvs_handles.dft)},
        .time{move(time)},
        .mdns{move(mdns)},
        .led{move(led)},
        .runtime{move(runtime)},
    };
    return App{move(handles)};
}

void App::run() { handles.runtime->run(); }
