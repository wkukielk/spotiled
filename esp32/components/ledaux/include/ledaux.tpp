#pragma once

namespace led {
template <I_LED LED>
Channel<LED>::Channel(channel_t channel, gpio_num_t pin)
    : channel_{channel}, pin_{pin} {}

template <I_LED LED> void Channel<LED>::configure() {
    auto ch_config =
        ledc_channel_config_t{.gpio_num = pin_,
                              .speed_mode = led::PWM::MODE,
                              .channel = channel_,
                              .intr_type = led::PWM::INTERRUPT_F,
                              .timer_sel = led::PWM::TIMER,
                              .duty = 0,
                              .hpoint = 0,
                              .sleep_mode = led::PWM::SLEEP_MODE,
                              .flags{.output_invert = led::PWM::INVERT}};
    auto err = LED::get().configure_channel(&ch_config);
    UTIL_THROW(err, InitError, "pwm channel config");
}

template <I_LED LED> void Channel<LED>::register_callback() {
    auto mode = led::PWM::MODE;
    auto err = LED::get().cb_register(mode, channel_, &led::fade_cb_s, nullptr);
    UTIL_THROW(err, InitError, "ledc cb register");
}

template <I_LED LED>
esp_err_t Channel<LED>::fade(uint32_t target_duty) noexcept {
    auto dur = led::PWM::DEFAULT_FADE_TIME_MS;
    return LED::get().set_fade_time_and_start(
        led::PWM::MODE, channel_, target_duty, dur, LEDC_FADE_NO_WAIT);
}

template <I_LED LED> uint32_t Channel<LED>::get_duty() const {
    uint32_t duty{};
    auto err = LED::get().get_duty(led::PWM::MODE, channel_, std::ref(duty));
    UTIL_THROW(err, RuntimeError, "channel get_duty");
    return duty;
}
} // namespace led

template <I_LED LED>
LEDC<LED>::LEDC() noexcept
    : red_{led::PWM::channel::RED, led::pin::RED},
      green_{led::PWM::channel::GREEN, led::pin::GREEN},
      blue_{led::PWM::channel::BLUE, led::pin::BLUE},
      ww_{led::PWM::channel::WW, led::pin::WW} {}

template <I_LED LED> esp_err_t LEDC<LED>::set_color(RGB rgb) noexcept {
    auto target = RGBDuty::rgb_to_duty(rgb);
    UTIL_RETURN(red_.fade(target.r_duty));
    UTIL_RETURN(green_.fade(target.g_duty));
    UTIL_RETURN(blue_.fade(target.b_duty));
    return ESP_OK;
}

template <I_LED LED> RGB LEDC<LED>::get_current_color() const noexcept {
    auto duty = RGBDuty{red_.get_duty(), green_.get_duty(), blue_.get_duty()};
    return duty.to_rgb();
}

template <I_LED LED> void LEDC<LED>::configure_pwm() {
    auto t_config =
        ledc_timer_config_t{led::PWM::MODE, led::PWM::RESOLUTION,
                            led::PWM::TIMER, led::PWM::FREQ, LED::CLOCK};
    UTIL_THROW(LED::get().configure_timer(&t_config), InitError, "ledc config");
    red_.configure();
    green_.configure();
    blue_.configure();
    ww_.configure();
    UTIL_THROW(LED::get().fade_func_install(0), InitError, "fade func install");
    red_.register_callback();
    green_.register_callback();
    blue_.register_callback();
    ww_.register_callback();
}
