#include "LED.hpp"

esp_err_t LED::configure_channel(const channel_config_t *cfg) noexcept {
    return ledc_channel_config(cfg);
}

esp_err_t LED::configure_timer(const timer_config_t *cfg) noexcept {
    return ledc_timer_config(cfg);
}

uint32_t LED::get_duty(mode_t speed_mode, channel_t channel) const noexcept {
    return ledc_get_duty(speed_mode, channel);
}

esp_err_t LED::fade_func_install(int intr_alloc_flags) noexcept {
    return ledc_fade_func_install(intr_alloc_flags);
}

esp_err_t LED::set_fade_time_and_start(ledc_mode_t speed_mode,
                                       channel_t channel, uint32_t target_duty,
                                       uint32_t desired_fade_time_ms,
                                       fade_mode_t fade_mode) noexcept {
    return ledc_set_fade_time_and_start(speed_mode, channel, target_duty,
                                        desired_fade_time_ms, fade_mode);
}

esp_err_t LED::cb_register(mode_t speed_mode, channel_t channel,
                           ledc_cbs_t *cbs, void *user_arg) noexcept {
    return ledc_cb_register(speed_mode, channel, cbs, user_arg);
}
