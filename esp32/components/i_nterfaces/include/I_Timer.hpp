#ifndef I_NTERFACES_TIMER_HPP
#define I_NTERFACES_TIMER_HPP

#include "esp_err.h"

#include <chrono>
#include <concepts>
#include <functional>
#include <optional>

namespace timer {
using callback = std::function<void()>;

// Distinguishes the timer kinds, which otherwise have identical signatures.
enum class kind { periodic, one_shot };
} // namespace timer

template <typename T>
concept I_TimerBase =
    std::constructible_from<T, timer::callback, const char *> &&
    !std::copyable<T> && !std::movable<T> && requires(T t, const T &ct) {
        { T::kind } -> std::convertible_to<timer::kind>;
        { t.stop() } -> std::same_as<esp_err_t>;
        { ct.is_active() } noexcept -> std::same_as<bool>;
    };

template <typename T>
concept I_PeriodicTimer =
    I_TimerBase<T> && (T::kind == timer::kind::periodic) &&
    requires(T t, std::chrono::microseconds period) {
        { t.start(period) } -> std::same_as<esp_err_t>;
        { t.restart(period) } -> std::same_as<esp_err_t>;
    };

template <typename T>
concept I_OneShotTimer =
    I_TimerBase<T> && (T::kind == timer::kind::one_shot) &&
    requires(T t, const T &ct, std::chrono::microseconds timeout) {
        { t.start(timeout) } -> std::same_as<esp_err_t>;
        { t.restart(timeout) } -> std::same_as<esp_err_t>;
        {
            ct.remaining()
        } noexcept -> std::same_as<std::optional<std::chrono::microseconds>>;
    };

#endif
