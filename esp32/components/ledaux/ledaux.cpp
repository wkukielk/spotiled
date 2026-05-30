#include "ledaux.hpp"
#include "esp_attr.h"

using namespace led;

namespace led {
bool IRAM_ATTR fade_callback(const ledc_cb_param_t *param, void *user_arg) {
    (void)param;
    (void)user_arg;
    return false;
}

ledc_cbs_t fade_cb_s{&fade_callback};

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

const ledc_timer_t TIMER = LEDC_TIMER_0;
const ledc_mode_t MODE = LEDC_LOW_SPEED_MODE;
const ledc_intr_type_t INTERRUPT_F = LEDC_INTR_DISABLE;
const ledc_sleep_mode_t SLEEP_MODE = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD;

const ledc_timer_bit_t RESOLUTION = LEDC_TIMER_8_BIT;
const int MAX_DUTY = (1 << RESOLUTION) - 1;
}; // namespace PWM
} // namespace led

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
