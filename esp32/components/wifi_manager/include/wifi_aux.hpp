#ifndef WIFI_AUX_HPP
#define WIFI_AUX_HPP

#include "I_WiFiProv.hpp"
#include "errors.hpp"
#include "util.hpp"
#include <functional>

namespace wifi {
using HandlerGetter = std::function<util::EventHandler *(util::I_EventGroup &)>;
using ProvGetter = std::function<I_WiFiProv *()>;
int await_connection(util::I_EventGroup &event_group);
} // namespace wifi

struct WiFiManagerError : public BaseError {
    WiFiManagerError(int reason, const char *msg)
        : BaseError("WiFiManagerError", msg, reason) {};
    WiFiManagerError(int reason) : BaseError("WiFiManagerError", "", reason) {};
};

#endif
