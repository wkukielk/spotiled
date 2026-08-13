#pragma once

namespace led {

template <I_LED LED>
ChannelConfig<LED>::ChannelConfig(ledc_channel_t channel, gpio_num_t pin)
    : channel_{channel}, pin_{pin} {
    auto ch_config =
        ledc_channel_config_t{.gpio_num = pin_,
                              .speed_mode = led::PWM::MODE,
                              .channel = channel_,
                              .timer_sel = led::PWM::TIMER,
                              .duty = 0,
                              .hpoint = 0,
                              .sleep_mode = led::PWM::SLEEP_MODE,
                              .flags{.output_invert = led::PWM::INVERT}};
    auto err = Handle<LED>::get().configure_channel(&ch_config);
    UTIL_THROW(err, InitError, "ledc channel config");
}

template <I_LED LED> ChannelConfig<LED>::~ChannelConfig() noexcept {
    auto ch_config = ledc_channel_config_t{.gpio_num = pin_,
                                           .speed_mode = led::PWM::MODE,
                                           .channel = channel_,
                                           .deconfigure = true};
    if (Handle<LED>::get().configure_channel(&ch_config) != ESP_OK)
        ESP_LOGW("ledaux", "channel deconfig failed");
}

template <I_LED LED>
Channel<LED>::Channel(channel_t channel, gpio_num_t pin)
    : channel_{channel}, config_{channel, pin} {}

template <I_LED LED>
esp_err_t Channel<LED>::fade(uint32_t target_duty,
                             uint32_t duration_ms) noexcept {
    UTIL_RETURN(Handle<LED>::get().stop_fade(led::PWM::MODE, channel_));
    return Handle<LED>::get().set_fade_time_and_start(
        led::PWM::MODE, channel_, target_duty, duration_ms, LEDC_FADE_NO_WAIT);
}

template <I_LED LED>
esp_err_t Channel<LED>::set_color(uint32_t target_duty) noexcept {
    UTIL_RETURN(Handle<LED>::get().stop_fade(led::PWM::MODE, channel_));
    UTIL_RETURN(
        Handle<LED>::get().set_duty(led::PWM::MODE, channel_, target_duty));
    return Handle<LED>::get().update_duty(led::PWM::MODE, channel_);
}

template <I_LED LED> uint32_t Channel<LED>::get_duty() const {
    uint32_t duty{};
    auto err =
        Handle<LED>::get().get_duty(led::PWM::MODE, channel_, std::ref(duty));
    UTIL_THROW(err, RuntimeError, "channel get_duty");
    return duty;
}

} // namespace led

template <I_LED LED>
esp_err_t LEDCConfigurer<LED>::configure_timer(Mode mode) noexcept {
    auto t_config = ledc_timer_config_t{
        led::PWM::MODE, led::PWM::RESOLUTION, led::PWM::TIMER,
        led::PWM::FREQ, LED::CLOCK,           mode == Mode::deconfig};
    return Handle<LED>::get().configure_timer(&t_config);
}

template <I_LED LED> LEDCConfigurer<LED>::LEDCConfigurer() {
    UTIL_THROW(this->configure_timer(Mode::config), InitError, "timer config");
    if (Handle<LED>::get().fade_func_install(0) != ESP_OK)
        ESP_LOGW("LEDC", "fade func install fail");
}

template <I_LED LED> LEDCConfigurer<LED>::~LEDCConfigurer() {
    if (this->configure_timer(Mode::deconfig) != ESP_OK)
        ESP_LOGW("LEDC", "timer deconfig fail");
    Handle<LED>::get().fade_func_uninstall();
}

template <I_LED LED> bool is_timer_configured_ = false;
template <I_LED LED> bool is_fade_func_installed_ = false;

template <I_LED LED>
LEDC<LED>::LEDC() noexcept
    : red_{led::PWM::channel::RED, led::pin::RED},
      green_{led::PWM::channel::GREEN, led::pin::GREEN},
      blue_{led::PWM::channel::BLUE, led::pin::BLUE},
      ww_{led::PWM::channel::WW, led::pin::WW} {}

template <I_LED LED>
esp_err_t LEDC<LED>::fade_to(RGB rgb, uint32_t duration_ms) noexcept {
    auto target = RGBDuty::rgb_to_duty(rgb);
    UTIL_RETURN(red_.fade(target.r_duty, duration_ms));
    UTIL_RETURN(green_.fade(target.g_duty, duration_ms));
    UTIL_RETURN(blue_.fade(target.b_duty, duration_ms));
    return ESP_OK;
}

template <I_LED LED> esp_err_t LEDC<LED>::set_color(RGB rgb) noexcept {
    auto target = RGBDuty::rgb_to_duty(rgb);
    UTIL_RETURN(red_.set_color(target.r_duty));
    UTIL_RETURN(green_.set_color(target.g_duty));
    UTIL_RETURN(blue_.set_color(target.b_duty));
    return ESP_OK;
}

template <I_LED LED> RGB LEDC<LED>::get_current_color() const noexcept {
    auto duty = RGBDuty{red_.get_duty(), green_.get_duty(), blue_.get_duty()};
    return duty.to_rgb();
}
