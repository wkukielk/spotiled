#ifndef I_GPIO_HPP
#define I_GPIO_HPP

#include "driver/gpio.h"

#include <concepts>
#include <type_traits>

namespace gpio {

enum class pull { none, down, up };

struct cfg {
    gpio_mode_t mode;
    gpio_pullup_t pull_up_en;
    gpio_pulldown_t pull_down_en;
};

enum class level { low = 0, high = 1 };

} // namespace gpio

template <typename T>
concept I_GPIO_IN = requires(const T &ct) {
    { ct.get_level() } noexcept -> std::same_as<gpio::level>;
} && std::is_constructible_v<T, gpio_num_t, gpio::pull>;

template <typename T>
concept I_GPIO_OUT = requires(T t, const T &ct, gpio::level level) {
    { t.set_level(level) } -> std::same_as<void>;
} && std::is_constructible_v<T, gpio_num_t>;

#endif
