#include "runtime.hpp"

static const char *TAG = "Runtime";

Runtime::Runtime(u_ptr<I_WiFiManager> wifi_mgr)
    : wifi_mgr{std::move(wifi_mgr)} {}

void Runtime::run() {}

void Runtime::setup() {
    UTIL_CHECK(wifi_mgr->init(), "failed to init wifi mgr");
    UTIL_CHECK(wifi_mgr->connect(), "Failed to connect to WiFi");
    ESP_LOGI(TAG, "Connected to WiFi");
}
