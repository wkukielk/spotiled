#include "Timer.hpp"
#include "errors.hpp"
#include "util.hpp"

#include <algorithm>
#include <utility>

static const char *TAG = "Timer";

using namespace std::chrono;

namespace timer::detail {

EspTimer::EspTimer(timer::callback cb, const char *name) : cb{std::move(cb)} {
    const esp_timer_create_args_t args{
        .callback = &EspTimer::dispatch,
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = name,
        .skip_unhandled_events = true,
    };
    UTIL_THROW(esp_timer_create(&args, &handle), InitError, "timer create");
}

EspTimer::~EspTimer() noexcept {
    if (is_active() && esp_timer_stop(handle) != ESP_OK)
        UTIL_LOGW("timer stop");
    if (esp_timer_delete(handle) != ESP_OK)
        UTIL_LOGW("timer delete");
}

esp_err_t EspTimer::stop() noexcept { return esp_timer_stop(handle); }

bool EspTimer::is_active() const noexcept {
    return esp_timer_is_active(handle);
}

void EspTimer::dispatch(void *arg) {
    auto *self = static_cast<EspTimer *>(arg);
    if (self->cb)
        self->cb();
}

} // namespace timer::detail

esp_err_t PeriodicTimer::start(microseconds period) noexcept {
    return esp_timer_start_periodic(handle, period.count());
}

esp_err_t PeriodicTimer::restart(microseconds period) noexcept {
    return esp_timer_restart(handle, period.count());
}

esp_err_t OneShotTimer::start(microseconds timeout) noexcept {
    return esp_timer_start_once(handle, timeout.count());
}

esp_err_t OneShotTimer::restart(microseconds timeout) noexcept {
    return esp_timer_restart(handle, timeout.count());
}

std::optional<microseconds> OneShotTimer::remaining() const noexcept {
    if (!is_active())
        return std::nullopt;
    uint64_t expiry{};
    if (esp_timer_get_expiry_time(handle, &expiry) != ESP_OK)
        return std::nullopt;
    auto left = static_cast<int64_t>(expiry) - esp_timer_get_time();
    return microseconds{std::max<int64_t>(left, 0)};
}
