#include "global.hpp"
#include "errors.hpp"
#include "esp_netif.h"
#include "util.hpp"

Netif::Netif() {
    auto err = esp_netif_init();
    UTIL_THROW(err, InitError, "netif init");
}

Netif::~Netif() {
    auto err = esp_netif_deinit();
    if (err != ESP_OK)
        ESP_LOGW("Netif", "netif deinit error: %d", err);
}

std::unique_ptr<I_Time> Global::time{};
std::unique_ptr<I_NVS> Global::nvs{};
std::unique_ptr<I_mDNSFacade> Global::mdns{};

void Global::set_nvs(std::unique_ptr<I_NVS> &&nvs) {
    Global::nvs = std::move(nvs);
}

void Global::set_time(std::unique_ptr<I_Time> &&time) {
    Global::time = std::move(time);
}

void Global::set_mdns(std::unique_ptr<I_mDNSFacade> &&mdns) {
    Global::mdns = std::move(mdns);
}
