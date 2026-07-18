#ifndef I_LED_HPP
#define I_LED_HPP

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"

#include <concepts>
#include <functional>

template <typename T>
concept I_LED =
    requires(T t, const T &ct, const ledc_channel_config_t *ch_cfg,
             const ledc_timer_config_t *timer_cfg, ledc_mode_t speed_mode,
             ledc_channel_t channel, uint32_t duty, uint32_t fade_time_ms,
             ledc_fade_mode_t fade_mode, int intr_alloc_flags) {
        { T::get() } -> std::same_as<T &>;

        { t.configure_channel(ch_cfg) } noexcept -> std::same_as<esp_err_t>;
        { t.configure_timer(timer_cfg) } noexcept -> std::same_as<esp_err_t>;
        {
            ct.get_duty(speed_mode, channel, std::ref(duty))
        } noexcept -> std::same_as<esp_err_t>;
        {
            t.fade_func_install(intr_alloc_flags)
        } noexcept -> std::same_as<esp_err_t>;
        { t.fade_func_uninstall() } noexcept -> std::same_as<void>;
        {
            t.set_fade_time_and_start(speed_mode, channel, duty, fade_time_ms,
                                      fade_mode)
        } noexcept -> std::same_as<esp_err_t>;
        { t.stop_fade(speed_mode, channel) } noexcept -> std::same_as<esp_err_t>;
        {
            t.set_duty(speed_mode, channel, duty)
        } noexcept -> std::same_as<esp_err_t>;
        { t.update_duty(speed_mode, channel) } noexcept -> std::same_as<esp_err_t>;
        requires std::same_as<decltype(T::CLOCK), const ledc_clk_cfg_t>;
    };

#endif
