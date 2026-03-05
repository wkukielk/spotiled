#include "ledaux.hpp"
#include "esp_attr.h"
#include "esp_check.h"
#include "esp_err.h"
#include <cinttypes>

namespace led {

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

esp_err_t cfg_pwm_channel(ledc_channel_t channel_num, gpio_num_t pin) {
    auto ch_config = default_ch_cfg;
    ch_config.gpio_num = pin;
    ch_config.channel = channel_num;
    return ledc_channel_config(&ch_config);
}

struct RGBDuty {
    uint32_t r_duty;
    uint32_t g_duty;
    uint32_t b_duty;
    RGB to_rgb();
    static RGBDuty rgb_to_duty(RGB);
};

RGBDuty get_current_duty() {
    auto curr_r = ledc_get_duty(PWM::MODE, PWM::channel::RED);
    auto curr_g = ledc_get_duty(PWM::MODE, PWM::channel::GREEN);
    auto curr_b = ledc_get_duty(PWM::MODE, PWM::channel::BLUE);
    return {curr_r, curr_g, curr_b};
}

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

esp_err_t configure_pwm() {
    auto t_config = ledc_timer_config_t{PWM::MODE, PWM::RESOLUTION, PWM::TIMER,
                                        PWM::FREQ, PWM::CLOCK};

    ESP_RETURN_ON_ERROR(ledc_timer_config(&t_config), "", "");
    ESP_RETURN_ON_ERROR(cfg_pwm_channel(PWM::channel::RED, pin::RED), "", "");
    ESP_RETURN_ON_ERROR(cfg_pwm_channel(PWM::channel::GREEN, pin::GREEN), "",
                        "");
    ESP_RETURN_ON_ERROR(cfg_pwm_channel(PWM::channel::BLUE, pin::BLUE), "", "");
    ESP_RETURN_ON_ERROR(cfg_pwm_channel(PWM::channel::WW, pin::WW), "", "");

    ESP_RETURN_ON_ERROR(ledc_fade_func_install(0), "", "");
    ESP_RETURN_ON_ERROR(
        ledc_cb_register(PWM::MODE, PWM::channel::RED, &fade_cb_s, nullptr), "",
        "");
    ESP_RETURN_ON_ERROR(
        ledc_cb_register(PWM::MODE, PWM::channel::GREEN, &fade_cb_s, nullptr),
        "", "");
    ESP_RETURN_ON_ERROR(
        ledc_cb_register(PWM::MODE, PWM::channel::BLUE, &fade_cb_s, nullptr),
        "", "");

    return ESP_OK;
}

RGB get_current_color() {
    auto duty = get_current_duty();
    return duty.to_rgb();
}

esp_err_t set_color(RGB rgb) {
    auto target_duty = RGBDuty::rgb_to_duty(rgb);
    ESP_RETURN_ON_ERROR(ledc_set_fade_time_and_start(
                            PWM::MODE, PWM::channel::RED, target_duty.r_duty,
                            PWM::DEFAULT_FADE_TIME_MS, LEDC_FADE_NO_WAIT),
                        "", "");
    ESP_RETURN_ON_ERROR(ledc_set_fade_time_and_start(
                            PWM::MODE, PWM::channel::GREEN, target_duty.g_duty,
                            PWM::DEFAULT_FADE_TIME_MS, LEDC_FADE_NO_WAIT),
                        "", "");
    ESP_RETURN_ON_ERROR(ledc_set_fade_time_and_start(
                            PWM::MODE, PWM::channel::BLUE, target_duty.b_duty,
                            PWM::DEFAULT_FADE_TIME_MS, LEDC_FADE_NO_WAIT),
                        "", "");
    return ESP_OK;
}
} // namespace led
