#ifndef LED_WRAP_HPP
#define LED_WRAP_HPP

#include "I_LED.hpp"

class LED : public I_LED {
    using I_LED::channel_config_t;
    using I_LED::channel_t;
    using I_LED::fade_mode_t;
    using I_LED::mode_t;
    using I_LED::timer_config_t;

  public:
    esp_err_t configure_channel(const channel_config_t *cfg) noexcept override;
    esp_err_t configure_timer(const timer_config_t *cfg) noexcept override;
    uint32_t get_duty(mode_t speed_mode,
                      channel_t channel) const noexcept override;
    esp_err_t fade_func_install(int intr_alloc_flags) noexcept override;
    esp_err_t set_fade_time_and_start(ledc_mode_t speed_mode, channel_t channel,
                                      uint32_t target_duty,
                                      uint32_t desired_fade_time_ms,
                                      fade_mode_t fade_mode) noexcept override;
    esp_err_t cb_register(mode_t speed_mode, channel_t channel, ledc_cbs_t *cbs,
                          void *user_arg) noexcept override;
};

#endif
