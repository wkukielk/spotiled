#include "global.hpp"
#include "errors.hpp"
#include "esp_netif.h"
#include "util.hpp"

Netif::Netif() { UTIL_THROW(esp_netif_init(), InitError{"netif"}); }

Netif::~Netif() noexcept { esp_netif_deinit(); }

DefaultEventLoop::DefaultEventLoop() {
    UTIL_THROW(esp_event_loop_create_default(),
               InitError{"default event loop"});
}

DefaultEventLoop::~DefaultEventLoop() noexcept {
    esp_event_loop_delete_default();
}

Global *Global::instance{};

Global::Global()
    : eloop{std::make_unique<DefaultEventLoop>()},
      netif{std::make_unique<Netif>()} {}

void Global::set_nvs(std::unique_ptr<I_NVS> &&nvs) {
    this->nvs = std::move(nvs);
}

void Global::set_wifi(std::unique_ptr<I_WiFi> &&wifi) {
    this->wifi = std::move(wifi);
}

void Global::set_time(std::unique_ptr<I_Time> &&time) {
    this->time = std::move(time);
}

Global *Global::get_global() {
    if (!instance)
        instance = new Global{};
    return instance;
}

void Global::deinit() {
    if (instance)
        delete instance;
}