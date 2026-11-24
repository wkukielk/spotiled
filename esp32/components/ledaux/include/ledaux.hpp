#ifndef LEDAUX_HPP
#define LEDAUX_HPP

#include "Handle.hpp"
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

template <I_LED LED> class ChannelConfig {
  public:
    ChannelConfig(ledc_channel_t channel, gpio_num_t pin);
    ~ChannelConfig() noexcept;

  private:
    ledc_channel_t channel_;
    gpio_num_t pin_;
};

template <I_LED LED> class Channel {
  public:
    Channel(ledc_channel_t channel, gpio_num_t pin);
    esp_err_t fade(uint32_t target_duty, uint32_t duration_ms) noexcept;
    esp_err_t set_color(uint32_t target_duty) noexcept;
    uint32_t get_duty() const;

  private:
    ledc_channel_t channel_;
    ChannelConfig<LED> config_;
    LED &led;
    const ledc_mode_t mode = led::PWM::MODE;
};
}; // namespace led

template <I_LED LED> class LEDCConfigurer {
  public:
    LEDCConfigurer();
    ~LEDCConfigurer();

  private:
    enum class Mode { config, deconfig };
    static esp_err_t configure_timer(Mode mode) noexcept;
};

template <typename T>
concept I_LEDC = std::default_initializable<T> &&
                 requires(T t, const T &ct, RGB rgb, uint32_t duration_ms) {
                     {
                         t.fade_to(rgb, duration_ms)
                     } noexcept -> std::same_as<esp_err_t>;
                     { t.set_color(rgb) } noexcept -> std::same_as<esp_err_t>;
                     { ct.get_current_color() } noexcept -> std::same_as<RGB>;
                 };

template <I_LED LED> class LEDC {
  public:
    LEDC() noexcept;
    esp_err_t fade_to(RGB rgb, uint32_t duration_ms) noexcept;
    esp_err_t set_color(RGB rgb) noexcept;
    RGB get_current_color() const noexcept;

    LEDCConfigurer<LED> configurer{};
    led::Channel<LED> red_;
    led::Channel<LED> green_;
    led::Channel<LED> blue_;
    led::Channel<LED> ww_;
};

#include "ledaux.tpp"

#endif
