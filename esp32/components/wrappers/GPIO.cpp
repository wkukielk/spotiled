#include "GPIO.hpp"
#include "errors.hpp"
#include "util.hpp"

static const char *TAG = "GPIO";

using namespace gpio;

GPIO_IN::GPIO_IN(gpio_num_t pin, gpio::pull pull) : pin{pin} {
    auto up = pull == pull::up ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
    auto dn = pull == pull::down ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE;
    auto cfg = gpio_config_t{.pin_bit_mask = (1ull << pin),
                             .mode = GPIO_MODE_INPUT,
                             .pull_up_en = up,
                             .pull_down_en = dn,
                             .intr_type = GPIO_INTR_DISABLE};
    UTIL_THROW(gpio_config(&cfg), InitError, "gpio_in init");
}

GPIO_IN::~GPIO_IN() noexcept {
    if (gpio_reset_pin(pin) != ESP_OK)
        UTIL_LOGW("gpio_in deinit");
}

level GPIO_IN::get_level() const noexcept { return (level)gpio_get_level(pin); }

GPIO_OUT::GPIO_OUT(gpio_num_t pin) : pin{pin} {
    auto cfg = gpio_config_t{.pin_bit_mask = (1ull << pin),
                             .mode = GPIO_MODE_INPUT,
                             .pull_up_en = GPIO_PULLUP_DISABLE,
                             .pull_down_en = GPIO_PULLDOWN_DISABLE,
                             .intr_type = GPIO_INTR_DISABLE};
    UTIL_THROW(gpio_config(&cfg), InitError, "gpio_out init");
}

GPIO_OUT::~GPIO_OUT() noexcept {
    if (gpio_reset_pin(pin) != ESP_OK)
        UTIL_LOGW("gpio_out deinit");
}

void GPIO_OUT::set_level(level level) {
    auto err = gpio_set_level(pin, (uint32_t)level);
    UTIL_THROW(err, RuntimeError, "gpio set level");
}
