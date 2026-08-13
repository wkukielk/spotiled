#ifndef MOCK_LED_HPP
#define MOCK_LED_HPP

#include "I_LED.hpp"
#include <gmock/gmock.h>

class MockLED {
  public:
    esp_err_t init() noexcept { return ESP_OK; }
    esp_err_t deinit() noexcept { return ESP_OK; }

    MOCK_METHOD(esp_err_t, configure_channel,
                (const ledc_channel_config_t *cfg), (noexcept));
    MOCK_METHOD(esp_err_t, configure_timer, (const ledc_timer_config_t *cfg),
                (noexcept));
    MOCK_METHOD(esp_err_t, get_duty,
                (ledc_mode_t speed_mode, ledc_channel_t channel,
                 uint32_t &duty),
                (const, noexcept));
    MOCK_METHOD(esp_err_t, fade_func_install, (int intr_alloc_flags),
                (noexcept));
    MOCK_METHOD(void, fade_func_uninstall, (), (noexcept));
    MOCK_METHOD(esp_err_t, set_fade_time_and_start,
                (ledc_mode_t speed_mode, ledc_channel_t channel,
                 uint32_t target_duty, uint32_t desired_fade_time_ms,
                 ledc_fade_mode_t fade_mode),
                (noexcept));
    MOCK_METHOD(esp_err_t, stop_fade,
                (ledc_mode_t speed_mode, ledc_channel_t channel), (noexcept));
    MOCK_METHOD(esp_err_t, set_duty,
                (ledc_mode_t speed_mode, ledc_channel_t channel,
                 uint32_t duty),
                (noexcept));
    MOCK_METHOD(esp_err_t, update_duty,
                (ledc_mode_t speed_mode, ledc_channel_t channel), (noexcept));
    MOCK_METHOD(bool, is_timer_configured, (), (noexcept));
    MOCK_METHOD(bool, is_fade_func_installed, (), (noexcept));

    static const ledc_clk_cfg_t CLOCK = (ledc_clk_cfg_t)0;
};

static_assert(I_LED<MockLED>);

#endif
