#pragma once

template <I_LED LED> LEDC<LED>::LEDC() noexcept : led_{LED{}} {}

template <I_LED LED> esp_err_t LEDC<LED>::set_color(RGB rgb) noexcept {
    auto target_duty = RGBDuty::rgb_to_duty(rgb);
    UTIL_RETURN(start_fade(led::PWM::channel::RED, target_duty.r_duty));
    UTIL_RETURN(start_fade(led::PWM::channel::GREEN, target_duty.g_duty));
    UTIL_RETURN(start_fade(led::PWM::channel::BLUE, target_duty.b_duty));
    return ESP_OK;
}

template <I_LED LED> RGB LEDC<LED>::get_current_color() const noexcept {
    return get_current_duty().to_rgb();
}

template <I_LED LED>
void LEDC<LED>::configure_pwm_channel(channel_t channel_num, gpio_num_t pin) {
    auto ch_config =
        ledc_channel_config_t{.gpio_num = pin,
                              .speed_mode = led::PWM::MODE,
                              .channel = channel_num,
                              .intr_type = led::PWM::INTERRUPT_F,
                              .timer_sel = led::PWM::TIMER,
                              .duty = 0,
                              .hpoint = 0,
                              .sleep_mode = led::PWM::SLEEP_MODE,
                              .flags{.output_invert = led::PWM::INVERT}};
    UTIL_THROW(led_.configure_channel(&ch_config), InitError, "pwm ch config");
}

template <I_LED LED> void LEDC<LED>::configure_pwm() {
    auto t_config =
        ledc_timer_config_t{led::PWM::MODE, led::PWM::RESOLUTION,
                            led::PWM::TIMER, led::PWM::FREQ, LED::CLOCK};
    UTIL_THROW(led_.configure_timer(&t_config), InitError, "ledc timer config");
    configure_pwm_channel(led::PWM::channel::RED, led::pin::RED);
    configure_pwm_channel(led::PWM::channel::GREEN, led::pin::GREEN);
    configure_pwm_channel(led::PWM::channel::BLUE, led::pin::BLUE);
    configure_pwm_channel(led::PWM::channel::WW, led::pin::WW);
    UTIL_THROW(led_.fade_func_install(0), InitError, "fade func install");
    register_callback(led::PWM::channel::RED);
    register_callback(led::PWM::channel::GREEN);
    register_callback(led::PWM::channel::BLUE);
    register_callback(led::PWM::channel::WW);
}

template <I_LED LED> void LEDC<LED>::register_callback(channel_t channel) {
    auto err =
        led_.cb_register(led::PWM::MODE, channel, &led::fade_cb_s, nullptr);
    UTIL_THROW(err, InitError, "ledc cb register");
}

template <I_LED LED>
esp_err_t LEDC<LED>::start_fade(channel_t channel,
                                uint32_t target_duty) noexcept {
    return led_.set_fade_time_and_start(led::PWM::MODE, channel, target_duty,
                                        led::PWM::DEFAULT_FADE_TIME_MS,
                                        LEDC_FADE_NO_WAIT);
}

template <I_LED LED> RGBDuty LEDC<LED>::get_current_duty() const noexcept {
    auto curr_r = led_.get_duty(led::PWM::MODE, led::PWM::channel::RED);
    auto curr_g = led_.get_duty(led::PWM::MODE, led::PWM::channel::GREEN);
    auto curr_b = led_.get_duty(led::PWM::MODE, led::PWM::channel::BLUE);
    return {curr_r, curr_g, curr_b};
}
