#ifndef I_LED_HPP
#define I_LED_HPP

#include "esp_err.h"
#include "ledc.h"

class I_LED {
  public:
    using channel_config_t = ledc_channel_config_t;
    using mode_t = ledc_mode_t;
    using channel_t = ledc_channel_t;
    using timer_config_t = ledc_timer_config_t;
    using fade_mode_t = ledc_fade_mode_t;

    virtual ~I_LED() noexcept {};
    virtual esp_err_t
    configure_channel(const channel_config_t *cfg) noexcept = 0;
    virtual esp_err_t configure_timer(const timer_config_t *cfg) noexcept = 0;
    virtual uint32_t get_duty(mode_t speed_mode,
                              channel_t channel) const noexcept = 0;
    virtual esp_err_t fade_func_install(int intr_alloc_flags) noexcept = 0;
    virtual esp_err_t
    set_fade_time_and_start(ledc_mode_t speed_mode, channel_t channel,
                            uint32_t target_duty, uint32_t desired_fade_time_ms,
                            fade_mode_t fade_mode) noexcept = 0;
    virtual esp_err_t cb_register(mode_t speed_mode, channel_t channel,
                                  ledc_cbs_t *cbs, void *user_arg) noexcept = 0;
};

#endif
