template <I_GPIO_OUT GPIO> Light<GPIO>::Light(gpio_num_t pin) : gpio{pin} {
    gpio.set_level(gpio::level::low);
}

template <I_GPIO_OUT GPIO> void Light<GPIO>::on() { set(true); }

template <I_GPIO_OUT GPIO> void Light<GPIO>::off() { set(false); }

template <I_GPIO_OUT GPIO> void Light<GPIO>::toggle() { set(!lit); }

template <I_GPIO_OUT GPIO> void Light<GPIO>::set(bool new_lit) {
    gpio.set_level(new_lit ? gpio::level::high : gpio::level::low);
    lit = new_lit;
}
