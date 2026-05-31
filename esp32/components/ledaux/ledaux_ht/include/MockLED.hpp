#ifndef MOCK_LED_HPP
#define MOCK_LED_HPP

#include "I_LED.hpp"
#include <gmock/gmock.h>

class MockLED {
  public:
    MOCK_METHOD(esp_err_t, configure_channel,
                (const ledc_channel_config_t *cfg));
    MOCK_METHOD(esp_err_t, configure_timer, (const ledc_timer_config_t *cfg));
    MOCK_METHOD(esp_err_t, get_duty,
                (ledc_mode_t speed_mode, ledc_channel_t channel,
                 uint32_t &duty),
                (const));
    MOCK_METHOD(esp_err_t, fade_func_install, (int intr_alloc_flags));
    MOCK_METHOD(esp_err_t, set_fade_time_and_start,
                (ledc_mode_t speed_mode, ledc_channel_t channel,
                 uint32_t target_duty, uint32_t desired_fade_time_ms,
                 ledc_fade_mode_t fade_mode));
    MOCK_METHOD(esp_err_t, cb_register,
                (ledc_mode_t speed_mode, ledc_channel_t channel,
                 ledc_cbs_t *cbs, void *user_arg));

    static const ledc_clk_cfg_t CLOCK = (ledc_clk_cfg_t)0;

    static MockLED &get() {
        if (!instance)
            instance = std::make_unique<MockLED>();
        return *instance;
    };
    static void reset() { instance = nullptr; };
    static MockLED &reset_and_get() {
        reset();
        return get();
    };
    static std::unique_ptr<MockLED> instance;
};

static_assert(I_LED<MockLED>);

#endif
