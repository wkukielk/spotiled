#include "init.hpp"
#include "errors.hpp"
#include "esp_netif.h"
#include "util.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "esp_event.h"
#pragma GCC diagnostic pop

Netif::Netif() { UTIL_THROW(esp_netif_init(), InitError{"netif"}); }

Netif::~Netif() noexcept { esp_netif_deinit(); }

DefaultEventLoop::DefaultEventLoop() {
    UTIL_THROW(esp_event_loop_create_default(),
               InitError{"default event loop"});
}

DefaultEventLoop::~DefaultEventLoop() noexcept {
    esp_event_loop_delete_default();
}