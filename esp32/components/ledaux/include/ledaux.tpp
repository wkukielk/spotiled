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
    auto err = LED::get().configure_channel(&ch_config);
    UTIL_THROW(err, InitError, "ledc channel config");
}

template <I_LED LED> ChannelConfig<LED>::~ChannelConfig() noexcept {
    auto ch_config = ledc_channel_config_t{.gpio_num = pin_,
                                           .speed_mode = led::PWM::MODE,
                                           .channel = channel_,
                                           .deconfigure = true};
    if (LED::get().configure_channel(&ch_config) != ESP_OK)
        ESP_LOGW("ledaux", "channel deconfig failed");
}

template <I_LED LED, DftEventLoopHandle DEL>
ChannelCallbacks<LED, DEL>::ChannelCallbacks(ledc_channel_t channel,
                                             PendingFade &pending)
    : channel_{channel} {
    auto err = LED::get().cb_register(led::PWM::MODE, channel_, &led::fade_cb_s,
                                      &pending);
    UTIL_THROW(err, InitError, "ledc cb register");
    err = DEL::get().handler_register(LEDC_FADE_EVENTS, (int32_t)channel_,
                                      &on_fade_event, this, &event_instance_);
    if (err != ESP_OK) {
        LED::get().cb_register(led::PWM::MODE, channel_, &led::fade_cb_null,
                               nullptr);
        throw InitError(err, "ledc fade event register");
    }
}

template <I_LED LED, DftEventLoopHandle DEL>
ChannelCallbacks<LED, DEL>::~ChannelCallbacks() noexcept {
    auto err = LED::get().cb_register(led::PWM::MODE, channel_,
                                      &led::fade_cb_null, nullptr);
    if (err != ESP_OK)
        ESP_LOGW("ledaux", "ledc cb unregister failed: %d", err);
    if (event_instance_) {
        auto err = DEL::get().handler_unregister(
            LEDC_FADE_EVENTS, (int32_t)channel_, event_instance_);
        if (err != ESP_OK)
            ESP_LOGW("ledaux", "ledc fade event unregister failed: %d", err);
    }
}

template <I_LED LED, DftEventLoopHandle DEL>
void ChannelCallbacks<LED, DEL>::on_fade_event(void *handler_arg,
                                               esp_event_base_t, int32_t,
                                               void *event_data) {
    auto *self = static_cast<ChannelCallbacks<LED, DEL> *>(handler_arg);
    auto duty = *static_cast<uint32_t *>(event_data);
    LED::get().set_fade_time_and_start(led::PWM::MODE, self->channel_, duty,
                                       led::PWM::DEFAULT_FADE_TIME_MS,
                                       LEDC_FADE_NO_WAIT);
}

template <I_LED LED, DftEventLoopHandle DEL>
Channel<LED, DEL>::Channel(channel_t channel, gpio_num_t pin)
    : channel_{channel}, config_{channel, pin}, callbacks_{channel, pending_} {}

template <I_LED LED, DftEventLoopHandle DEL>
esp_err_t Channel<LED, DEL>::fade(uint32_t target_duty) noexcept {
    pending_.duty.store((int32_t)target_duty);
    bool expected = false;
    if (!pending_.active.compare_exchange_strong(expected, true))
        return ESP_OK;
    auto duty = (uint32_t)pending_.duty.exchange(-1);
    return LED::get().set_fade_time_and_start(led::PWM::MODE, channel_, duty,
                                              led::PWM::DEFAULT_FADE_TIME_MS,
                                              LEDC_FADE_NO_WAIT);
}

template <I_LED LED, DftEventLoopHandle DEL>
uint32_t Channel<LED, DEL>::get_duty() const {
    uint32_t duty{};
    auto err = LED::get().get_duty(led::PWM::MODE, channel_, std::ref(duty));
    UTIL_THROW(err, RuntimeError, "channel get_duty");
    return duty;
}

} // namespace led

template <I_LED LED>
esp_err_t LEDCConfigurer<LED>::configure_timer(Mode mode) noexcept {
    auto t_config = ledc_timer_config_t{
        led::PWM::MODE, led::PWM::RESOLUTION, led::PWM::TIMER,
        led::PWM::FREQ, LED::CLOCK,           mode == Mode::deconfig};
    return LED::get().configure_timer(&t_config);
}

template <I_LED LED> LEDCConfigurer<LED>::LEDCConfigurer() {
    UTIL_THROW(this->configure_timer(Mode::config), InitError, "timer config");
    if (LED::get().fade_func_install(0) != ESP_OK)
        ESP_LOGW("LEDC", "fade func install fail");
}

template <I_LED LED> LEDCConfigurer<LED>::~LEDCConfigurer() {
    if (this->configure_timer(Mode::deconfig) != ESP_OK)
        ESP_LOGW("LEDC", "timer deconfig fail");
    LED::get().fade_func_uninstall();
}

template <I_LED LED> bool is_timer_configured_ = false;
template <I_LED LED> bool is_fade_func_installed_ = false;

template <I_LED LED, DftEventLoopHandle DEL>
LEDC<LED, DEL>::LEDC() noexcept
    : red_{led::PWM::channel::RED, led::pin::RED},
      green_{led::PWM::channel::GREEN, led::pin::GREEN},
      blue_{led::PWM::channel::BLUE, led::pin::BLUE},
      ww_{led::PWM::channel::WW, led::pin::WW} {}

template <I_LED LED, DftEventLoopHandle DEL>
esp_err_t LEDC<LED, DEL>::set_color(RGB rgb) noexcept {
    auto target = RGBDuty::rgb_to_duty(rgb);
    UTIL_RETURN(red_.fade(target.r_duty));
    UTIL_RETURN(green_.fade(target.g_duty));
    UTIL_RETURN(blue_.fade(target.b_duty));
    return ESP_OK;
}

template <I_LED LED, DftEventLoopHandle DEL>
RGB LEDC<LED, DEL>::get_current_color() const noexcept {
    auto duty = RGBDuty{red_.get_duty(), green_.get_duty(), blue_.get_duty()};
    return duty.to_rgb();
}
