#include "global.hpp"
#include "errors.hpp"
#include "esp_netif.h"
#include "util.hpp"

class Netif {
  public:
    static void init();
    static void deinit();
};

void Netif::init() {
    auto err = esp_netif_init();
    UTIL_THROW(err, InitError, "netif init");
}
void Netif::deinit() {
    auto err = esp_netif_deinit();
    UTIL_THROW(err, InitError, "netif deinit");
};

std::optional<Global> Global::instance{};

Global::Global() { Netif::init(); }

Global::~Global() noexcept { Netif::deinit(); }

void Global::set_nvs(std::unique_ptr<I_NVS> &&nvs) {
    this->nvs = std::move(nvs);
}

void Global::set_time(std::unique_ptr<I_Time> &&time) {
    this->time = std::move(time);
}

void Global::set_mdns(std::unique_ptr<I_mDNS> &&mdns) {
    this->mdns = std::move(mdns);
}

void Global::init() { instance.emplace(Global{}); }

Global &Global::get_global() { return *instance; }

void Global::deinit() { instance.reset(); }
