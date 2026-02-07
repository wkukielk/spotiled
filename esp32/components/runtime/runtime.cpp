#include "runtime.hpp"

static const char *TAG = "Runtime";

Runtime::Runtime(u_ptr<I_WiFiManager> wifi_mgr)
    : wifi_mgr{std::move(wifi_mgr)} {}

void Runtime::run() { setup(); }

void Runtime::setup() {
    UTIL_THROW(wifi_mgr->init(), InitError, "failed to init wifi mgr");
    UTIL_THROW(wifi_mgr->connect(), InitError, "Failed to connect to WiFi");
    ESP_LOGI(TAG, "Connected to WiFi");
}
