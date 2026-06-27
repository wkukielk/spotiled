#include "app.hpp"
#include "Configurer.hpp"
#include "WiFi.hpp"
#include "global.hpp"
#include "server/server.hpp"
#include "time_aux.hpp"
#include <memory>

using namespace std;

App::App(AppHandles &&handles) : handles{move(handles)} {}

App App::create() {
    auto cfgnvs = NVS{"cfg"};
    auto encnvs = NVS{"encnvs"};
    auto cfg_reader = Configurer{cfgnvs, encnvs};
    cfg_reader.preconfigure();

    auto eloop = Handle<DftEventLoop>{};
    auto netif = Netif{};
    auto nvs = NVSHandle{mkuptr<NVS>()};
    auto time = TimeHandle{mkuptr<Time>()};

    auto cfg = cfg_reader.get_config();
    auto mdns = MDNSHandle{move(cfg.mdns)};

    auto runtime = runtime_t::create(RuntimeConfig{
        .backend_cfg = cfg.backend_cfg, .wifi_mgr = move(cfg.wifi_mgr)});

    auto handles = AppHandles{
        .eloop{move(eloop)},
        .netif{move(netif)},
        .nvs{move(nvs)},
        .time{move(time)},
        .mdns{move(mdns)},
        .runtime{move(runtime)},
    };
    return App{move(handles)};
}

void App::run() { handles.runtime.run(); }
