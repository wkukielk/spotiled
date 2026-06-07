#include "global.hpp"
#include "errors.hpp"
#include "esp_netif.h"
#include "util.hpp"

class Netif {
  public:
    static void init();
    static void deinit();
};

class DefaultEventLoop {
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

void DefaultEventLoop::init() {
    auto err = esp_event_loop_create_default();
    UTIL_THROW(err, InitError, "default event loop create");
}
void DefaultEventLoop::deinit() {
    auto err = esp_event_loop_delete_default();
    UTIL_THROW(err, InitError, "default event loop delete");
};

Global *Global::instance{};

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

Global &Global::get_global() {
    if (!instance)
        instance = new Global{};
    return *instance;
}

void Global::deinit() {
    if (instance)
        delete instance;
    instance = nullptr;
}