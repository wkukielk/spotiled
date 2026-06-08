#ifndef WIFI_AUX_HPP
#define WIFI_AUX_HPP

#include "I_WiFi.hpp"
#include "I_WiFiProv.hpp"
#include "URL.hpp"
#include "errors.hpp"
#include "util.hpp"
#include <functional>
#include <optional>

namespace wifi {
using HandlerGetter =
    std::function<util::EventHandler *(util::EventGroupBase &)>;
int await_connection(util::EventGroupBase &event_group);

extern const EventBits_t fail_bit;
extern const EventBits_t success_bit;
extern const EventBits_t end_bit;
} // namespace wifi

struct WiFiManagerError : public BaseError {
    WiFiManagerError(int reason, const char *msg)
        : BaseError("WiFiManagerError", msg, reason) {};
    WiFiManagerError(int reason) : BaseError("WiFiManagerError", "", reason) {};
};

esp_err_t mdns_resolve(const std::string &hostname, std::string &out,
                       uint32_t timeout = 1000);

std::optional<URL> resolve_url(const URL &url, esp_err_t &err,
                               uint32_t timeout = 1000);

#endif
