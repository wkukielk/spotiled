#ifndef LED_WRAP_HPP
#define LED_WRAP_HPP

#include "I_LED.hpp"
#include "global.hpp"

class LED {
    LED();

  public:
    esp_err_t configure_channel(const ledc_channel_config_t *cfg) noexcept {
        return ledc_channel_config(cfg);
    }
    esp_err_t configure_timer(const ledc_timer_config_t *cfg) noexcept {
        return ledc_timer_config(cfg);
    }
    esp_err_t get_duty(ledc_mode_t speed_mode, ledc_channel_t channel,
                       uint32_t &duty) const noexcept {
        auto err = ledc_get_duty(speed_mode, channel);
        if (err == LEDC_ERR_DUTY)
            return err;
        duty = err;
        return ESP_OK;
    }
    esp_err_t fade_func_install(int intr_alloc_flags) noexcept {
        return ledc_fade_func_install(intr_alloc_flags);
    }
    void fade_func_uninstall() noexcept { return ledc_fade_func_uninstall(); }
    esp_err_t set_fade_time_and_start(ledc_mode_t speed_mode,
                                      ledc_channel_t channel,
                                      uint32_t target_duty,
                                      uint32_t desired_fade_time_ms,
                                      ledc_fade_mode_t fade_mode) noexcept {
        return ledc_set_fade_time_and_start(speed_mode, channel, target_duty,
                                            desired_fade_time_ms, fade_mode);
    }
    esp_err_t stop_fade(ledc_mode_t speed_mode,
                        ledc_channel_t channel) noexcept {
        return ledc_fade_stop(speed_mode, channel);
    }
    esp_err_t set_duty(ledc_mode_t speed_mode, ledc_channel_t channel,
                       uint32_t duty) noexcept {
        return ledc_set_duty(speed_mode, channel, duty);
    }
    esp_err_t update_duty(ledc_mode_t speed_mode,
                          ledc_channel_t channel) noexcept {
        return ledc_update_duty(speed_mode, channel);
    }

    static LED &get() { return instance; }

    static const ledc_clk_cfg_t CLOCK = LEDC_USE_APB_CLK;

  private:
    static LED instance;

    friend class Global;
};

static_assert(I_LED<LED>);

#endif
