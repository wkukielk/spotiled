#include "app.hpp"
#include "ConfigReader.hpp"
#include "WiFi.hpp"
#include "global.hpp"
#include "time_aux.hpp"
#include <memory>

const char *TAG = "APP";

App::App() : cfgnvs{std::make_unique<NVS>("cfg")} {
    Global::get_global()->set_nvs(mkuptr<NVS>());
    Global::get_global()->set_time(mkuptr<Time>());
    Global::get_global()->set_wifi(mkuptr<WiFi>(&wifi::cfg));
    auto cfg_reader = ConfigReader{*cfgnvs};
    auto cfg = cfg_reader.get_config();
    runtime = mkuptr<Runtime>(std::move(cfg.wifi_mgr));
}

void App::run() {
    UTIL_LOGI("hello");
    runtime->run();
    time().delay_ms(1e5);
}
