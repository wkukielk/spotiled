#ifndef LEDAUX_HPP
#define LEDAUX_HPP

#include "driver/ledc.h"
#include "rgb.hpp"

template <int8_t n> constexpr uint32_t rshift(uint32_t x) {
    static_assert(n > -32 && n < 32, "Shift amount must be between -31 and 31");
    if constexpr (n >= 0)
        return x >> n;
    else
        return x << -n;
}

template <int8_t n> constexpr uint32_t lshift(uint32_t x) {
    return rshift<-n>(x);
}

namespace led {

namespace pin {
extern const gpio_num_t RED;
extern const gpio_num_t GREEN;
extern const gpio_num_t BLUE;
extern const gpio_num_t WW;
}; // namespace pin

namespace PWM {

namespace channel {
extern const ledc_channel_t RED;
extern const ledc_channel_t GREEN;
extern const ledc_channel_t BLUE;
extern const ledc_channel_t WW;
} // namespace channel

inline constexpr ledc_clk_cfg_t CLOCK = LEDC_USE_APB_CLK;
inline constexpr ledc_timer_t TIMER = LEDC_TIMER_0;
inline constexpr ledc_mode_t MODE = LEDC_LOW_SPEED_MODE;
inline constexpr ledc_intr_type_t INTERRUPT_F = LEDC_INTR_DISABLE;
inline constexpr ledc_sleep_mode_t SLEEP_MODE = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD;

inline constexpr int FREQ = 1000; // Hz
inline constexpr ledc_timer_bit_t RESOLUTION = LEDC_TIMER_8_BIT;
inline constexpr int MAX_DUTY = (1 << RESOLUTION) - 1;

inline constexpr bool INVERT = false;
extern const int DEFAULT_FADE_TIME_MS;
}; // namespace PWM

esp_err_t configure_pwm();
RGB get_current_color();
esp_err_t set_color(RGB);

}; // namespace led

#endif
