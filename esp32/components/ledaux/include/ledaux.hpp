#ifndef LEDAUX_HPP
#define LEDAUX_HPP

#include "I_LED.hpp"
#include "errors.hpp"
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

extern const ledc_timer_t TIMER;
extern const ledc_mode_t MODE;
extern const ledc_intr_type_t INTERRUPT_F;
extern const ledc_sleep_mode_t SLEEP_MODE;

inline constexpr int FREQ = 1000; // Hz
extern const ledc_timer_bit_t RESOLUTION;
extern const int MAX_DUTY;

inline constexpr bool INVERT = false;
inline constexpr int DEFAULT_FADE_TIME_MS = 1000;
}; // namespace PWM
extern ledc_cbs_t fade_cb_s;
}; // namespace led

template <I_LED LED> class LEDC {
    using channel_t = ledc_channel_t;

  public:
    LEDC() noexcept;
    esp_err_t set_color(RGB rgb) noexcept;
    RGB get_current_color() const noexcept;

  private:
    void configure_pwm_channel(channel_t channel, gpio_num_t pin);
    void configure_pwm();
    void register_callback(channel_t channel);
    esp_err_t start_fade(channel_t channel, uint32_t target_duty) noexcept;
    RGBDuty get_current_duty() const noexcept;

  protected:
    LED led_;
};

#include "ledaux.tpp"

#endif
