#ifndef WIFI_AUX_HPP
#define WIFI_AUX_HPP

#include "I_WiFiProv.hpp"
#include "I_WiFi.hpp"
#include "URL.hpp"
#include "errors.hpp"
#include "util.hpp"
#include <functional>
#include <optional>

template<typename T>
concept WiFiManagerTypes = util::I_EventGroup<typename T::EventGroup> && I_WiFi<typename T::WiFi>;


namespace wifi {
using HandlerGetter = std::function<util::EventHandler *(util::EventGroupBase &)>;
using ProvGetter = std::function<I_WiFiProv *()>;
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
