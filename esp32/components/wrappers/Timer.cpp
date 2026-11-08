#include "Timer.hpp"
#include "errors.hpp"
#include "util.hpp"

#include <utility>

static const char *TAG = "Timer";

Timer::Timer(timer::callback cb, const char *name) : cb{std::move(cb)} {
    const esp_timer_create_args_t args{
        .callback = &Timer::dispatch,
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = name,
        .skip_unhandled_events = true,
    };
    UTIL_THROW(esp_timer_create(&args, &handle), InitError, "timer create");
}

Timer::~Timer() noexcept {
    if (is_active() && esp_timer_stop(handle) != ESP_OK)
        UTIL_LOGW("timer stop");
    if (esp_timer_delete(handle) != ESP_OK)
        UTIL_LOGW("timer delete");
}

esp_err_t Timer::start_once(std::chrono::microseconds timeout) noexcept {
    return esp_timer_start_once(handle, timeout.count());
}

esp_err_t Timer::start_periodic(std::chrono::microseconds period) noexcept {
    return esp_timer_start_periodic(handle, period.count());
}

esp_err_t Timer::restart(std::chrono::microseconds timeout) noexcept {
    return esp_timer_restart(handle, timeout.count());
}

esp_err_t Timer::stop() noexcept { return esp_timer_stop(handle); }

bool Timer::is_active() const noexcept { return esp_timer_is_active(handle); }

void Timer::dispatch(void *arg) {
    auto *self = static_cast<Timer *>(arg);
    if (self->cb)
        self->cb();
}
