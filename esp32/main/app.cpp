#include "app.hpp"
#include "Configurer.hpp"
#include "WiFi.hpp"
#include "global.hpp"
#include "time_aux.hpp"
#include <memory>

const char *TAG = "APP";

App::App() : cfgnvs{mkuptr<NVS>("cfg")} {
    Global::get_global()->set_nvs(mkuptr<NVS>());
    Global::get_global()->set_time(mkuptr<Time>());
    Global::get_global()->set_wifi(mkuptr<WiFi>(&wifi::cfg));
    auto cfg_reader = Configurer{*cfgnvs};
    cfg_reader.preconfigure();
    auto cfg = cfg_reader.get_config();
    Global::get_global()->set_mdns(std::move(cfg.mdns));
    runtime = mkuptr<Runtime>(std::move(cfg.wifi_mgr));
}

void App::run() {
    UTIL_LOGI("hello");
    runtime->run();
    time().delay_ms(1e5);
}
