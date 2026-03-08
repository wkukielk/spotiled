#ifndef LEDAUX_HPP
#define LEDAUX_HPP

#include "I_LED.hpp"
#include "rgb.hpp"
#include "util.hpp"

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

struct RGBDuty {
    uint32_t r_duty;
    uint32_t g_duty;
    uint32_t b_duty;
    RGB to_rgb();
    static RGBDuty rgb_to_duty(RGB);
};

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
}; // namespace led

class LEDC {
    using channel_config_t = ledc_channel_config_t;
    using mode_t = ledc_mode_t;
    using channel_t = ledc_channel_t;
    using timer_config_t = ledc_timer_config_t;
    using fade_mode_t = ledc_fade_mode_t;

  public:
    explicit LEDC(u_ptr<I_LED> &&led) noexcept;
    esp_err_t set_color(RGB rgb) noexcept;
    RGB get_current_color() const noexcept;

  private:
    void configure_pwm_channel(channel_t channel, gpio_num_t pin);
    void configure_pwm();
    void register_callback(channel_t channel);
    esp_err_t start_fade(channel_t channel, uint32_t target_duty) noexcept;
    RGBDuty get_current_duty() const noexcept;

    u_ptr<I_LED> led{};
};

#endif
