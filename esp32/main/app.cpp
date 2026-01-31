#include "app.hpp"
#include "ConfigReader.hpp"
#include "WiFi.hpp"
#include "global.hpp"
#include "time_aux.hpp"
#include <memory>

const char *TAG = "APP";

App::App() : cfgnvs{std::make_unique<NVS>("cfg")} {
    Global::get_global()->set_nvs(std::make_unique<NVS>());
    Global::get_global()->set_time(std::make_unique<Time>());
    Global::get_global()->set_wifi(std::make_unique<WiFi>(&wifi::cfg));
    auto cfg_reader = ConfigReader{*cfgnvs};
    cfg_reader.get_config();
}

void App::run() {
    UTIL_LOGI("hello");
    time().delay_ms(1e5);
}
