#ifndef I_NTERFACES_TIMER_HPP
#define I_NTERFACES_TIMER_HPP

#include "esp_err.h"

#include <chrono>
#include <concepts>
#include <functional>

namespace timer {
using callback = std::function<void()>;
} // namespace timer

template <typename T>
concept I_Timer = std::constructible_from<T, timer::callback, const char *> &&
                  !std::copyable<T> && !std::movable<T> &&
                  requires(T t, const T &ct, std::chrono::microseconds period) {
                      { t.start_once(period) } -> std::same_as<esp_err_t>;
                      { t.start_periodic(period) } -> std::same_as<esp_err_t>;
                      { t.restart(period) } -> std::same_as<esp_err_t>;
                      { t.stop() } -> std::same_as<esp_err_t>;
                      { ct.is_active() } noexcept -> std::same_as<bool>;
                  };

#endif
