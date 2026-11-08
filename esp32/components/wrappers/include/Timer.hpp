#ifndef WRAPPERS_TIMER_HPP
#define WRAPPERS_TIMER_HPP

#include "I_Timer.hpp"
#include "esp_timer.h"
#include <chrono>
#include <optional>
#include <utility>

namespace timer::detail {

// Owns the esp_timer handle; callback is dispatched from the esp_timer task.
class EspTimer {
  public:
    EspTimer(const EspTimer &) = delete;
    EspTimer &operator=(const EspTimer &) = delete;
    EspTimer(EspTimer &&) = delete;
    EspTimer &operator=(EspTimer &&) = delete;

    esp_err_t stop() noexcept;
    bool is_active() const noexcept;

  protected:
    EspTimer(timer::callback cb, const char *name);
    ~EspTimer() noexcept;

    esp_timer_handle_t handle{};

  private:
    static void dispatch(void *arg);

    timer::callback cb;
};

} // namespace timer::detail

class PeriodicTimer : public timer::detail::EspTimer {
  public:
    static constexpr timer::kind kind = timer::kind::periodic;

    PeriodicTimer(timer::callback cb, const char *name)
        : EspTimer{std::move(cb), name} {}

    esp_err_t start(std::chrono::microseconds period) noexcept;
    // Running timer only: restarts now with a new period.
    esp_err_t restart(std::chrono::microseconds period) noexcept;
};

class OneShotTimer : public timer::detail::EspTimer {
  public:
    static constexpr timer::kind kind = timer::kind::one_shot;

    OneShotTimer(timer::callback cb, const char *name)
        : EspTimer{std::move(cb), name} {}

    esp_err_t start(std::chrono::microseconds timeout) noexcept;
    // Running timer only: rearms to fire `timeout` from now.
    esp_err_t restart(std::chrono::microseconds timeout) noexcept;
    // Time left until it fires; nullopt if not running.
    std::optional<std::chrono::microseconds> remaining() const noexcept;
};

static_assert(I_PeriodicTimer<PeriodicTimer>);
static_assert(I_OneShotTimer<OneShotTimer>);
static_assert(!I_PeriodicTimer<OneShotTimer>);
static_assert(!I_OneShotTimer<PeriodicTimer>);

#endif
