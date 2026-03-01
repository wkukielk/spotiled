#include "BackendConfigurer.hpp"
#include "WiFi.hpp"
#include "WiFiManagerFactory.hpp"
#include "global.hpp"
#include "mDNSConfig.hpp"
#include "mytest_utils.hpp"
#include "unity.h"
#include "wifi_aux.hpp"

using namespace std;

namespace {
u_ptr<I_WiFiManager> wifi_mgr{};
optional<URL> host_url{};

void setup() {
    try {
        Global::get_global()->set_nvs(mkuptr<NVS>());
        Global::get_global()->set_time(mkuptr<Time>());
        Global::get_global()->set_wifi(mkuptr<WiFi>(&wifi::cfg));
        auto cfgnvs = NVS{"cfg"};
        auto mdns_config = mDNSConfig{cfgnvs};
        Global::get_global()->set_mdns(mdns_config.get());
        auto encnvs = NVS{"encnvs"};
        auto backend_config = BackendConfigurer{cfgnvs, encnvs};
        host_url = backend_config.get().base_url;

        auto mgr_factory = WiFiManagerFactory{cfgnvs};
        wifi_mgr = mgr_factory.get();
        wifi_mgr->init();
        wifi_mgr->connect();
    } catch (const std::exception &e) {
        ESP_LOGI("mdns_wrap_test", "%s", e.what());
    }
}

void teardown() {
    wifi_mgr.reset();
    Global::deinit();
    host_url.reset();
}

}; // namespace

TEST_CASE("setup", "[mdns_wrap]") {
    setup();
    disable_leak_check();
}

TEST_CASE("write read", "[mdns_wrap]") {
    set_leak_threshold(7000);
    auto err = esp_err_t{};
    auto url = resolve_url(host_url.value(), ref(err));
    TEST_ASSERT(url);
}

TEST_CASE("teardown", "[mdns_wrap]") { teardown(); }
