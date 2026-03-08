#include "ledaux.hpp"
#include "errors.hpp"
#include "esp_attr.h"
#include "esp_check.h"
#include "esp_err.h"
#include <cinttypes>

using namespace std;
using namespace led;

RGB RGBDuty::to_rgb() {
    auto rgb = RGB{};
    rgb.r = rshift<PWM::RESOLUTION - 8>(r_duty);
    rgb.g = rshift<PWM::RESOLUTION - 8>(g_duty);
    rgb.b = rshift<PWM::RESOLUTION - 8>(b_duty);
    return rgb;
}

RGBDuty RGBDuty::rgb_to_duty(RGB rgb) {
    return {lshift<PWM::RESOLUTION - 8>(rgb.r),
            lshift<PWM::RESOLUTION - 8>(rgb.g),
            lshift<PWM::RESOLUTION - 8>(rgb.b)};
}

namespace led {
bool IRAM_ATTR fade_callback(const ledc_cb_param_t *param, void *user_arg) {
    (void)param;
    (void)user_arg;
    return false; // something can be added
}

static ledc_cbs_t fade_cb_s{&fade_callback};

namespace pin {
const gpio_num_t RED = GPIO_NUM_3;
const gpio_num_t GREEN = GPIO_NUM_4;
const gpio_num_t BLUE = GPIO_NUM_5;
const gpio_num_t WW = GPIO_NUM_6;
}; // namespace pin

namespace PWM {
namespace channel {
const ledc_channel_t RED = LEDC_CHANNEL_0;
const ledc_channel_t GREEN = LEDC_CHANNEL_1;
const ledc_channel_t BLUE = LEDC_CHANNEL_2;
const ledc_channel_t WW = LEDC_CHANNEL_3;
} // namespace channel

const int DEFAULT_FADE_TIME_MS = 1000;
}; // namespace PWM
} // namespace led

using namespace led;

constexpr ledc_channel_config_t default_ch_cfg{
    .gpio_num = 0,
    .speed_mode = PWM::MODE,
    .channel = LEDC_CHANNEL_MAX,
    .intr_type = PWM::INTERRUPT_F,
    .timer_sel = PWM::TIMER,
    .duty = 0,
    .hpoint = 0,
    .sleep_mode = PWM::SLEEP_MODE,
    .flags{.output_invert = PWM::INVERT}};

LEDC::LEDC(u_ptr<I_LED> &&led) noexcept : led{move(led)} {}

void LEDC::configure_pwm_channel(channel_t channel_num, gpio_num_t pin) {
    auto ch_config = default_ch_cfg;
    ch_config.gpio_num = pin;
    ch_config.channel = channel_num;
    UTIL_THROW(led->configure_channel(&ch_config), InitError, "pwm ch config");
}

void LEDC::register_callback(channel_t channel) {
    auto err = led->cb_register(PWM::MODE, channel, &fade_cb_s, nullptr);
    UTIL_THROW(err, InitError, "ledc cb register");
}

void LEDC::configure_pwm() {
    auto t_config = ledc_timer_config_t{PWM::MODE, PWM::RESOLUTION, PWM::TIMER,
                                        PWM::FREQ, PWM::CLOCK};

    UTIL_THROW(led->configure_timer(&t_config), InitError, "ledc timer config");
    configure_pwm_channel(PWM::channel::RED, pin::RED);
    configure_pwm_channel(PWM::channel::GREEN, pin::GREEN);
    configure_pwm_channel(PWM::channel::BLUE, pin::BLUE);
    configure_pwm_channel(PWM::channel::WW, pin::WW);

    UTIL_THROW(led->fade_func_install(0), InitError, "fade func install");
    register_callback(PWM::channel::RED);
    register_callback(PWM::channel::GREEN);
    register_callback(PWM::channel::BLUE);
    register_callback(PWM::channel::WW);
}

esp_err_t LEDC::start_fade(channel_t channel, uint32_t target_duty) noexcept {
    return led->set_fade_time_and_start(PWM::MODE, channel, target_duty,
                                        PWM::DEFAULT_FADE_TIME_MS,
                                        LEDC_FADE_NO_WAIT);
}

esp_err_t LEDC::set_color(RGB rgb) noexcept {
    auto target_duty = RGBDuty::rgb_to_duty(rgb);
    UTIL_RETURN(start_fade(PWM::channel::RED, target_duty.r_duty));
    UTIL_RETURN(start_fade(PWM::channel::GREEN, target_duty.g_duty));
    UTIL_RETURN(start_fade(PWM::channel::BLUE, target_duty.b_duty));
    return ESP_OK;
}

RGB LEDC::get_current_color() const noexcept {
    auto duty = get_current_duty();
    return duty.to_rgb();
}

RGBDuty LEDC::get_current_duty() const noexcept {
    auto curr_r = led->get_duty(PWM::MODE, PWM::channel::RED);
    auto curr_g = led->get_duty(PWM::MODE, PWM::channel::GREEN);
    auto curr_b = led->get_duty(PWM::MODE, PWM::channel::BLUE);
    return {curr_r, curr_g, curr_b};
}
