template <I_GPIO_OUT GPIO, I_OneShotTimer Timer>
Light<GPIO, Timer>::Light(gpio_num_t pin)
    : gpio{pin}, timer{[this] { tick(); }, "light"} {
    gpio.set_level(gpio::level::low);
}

template <I_GPIO_OUT GPIO, I_OneShotTimer Timer>
void Light<GPIO, Timer>::on() noexcept {
    auto lock = std::lock_guard{m};
    blinking = false;
    timer.stop();
    set(true);
}

template <I_GPIO_OUT GPIO, I_OneShotTimer Timer>
void Light<GPIO, Timer>::off() noexcept {
    auto lock = std::lock_guard{m};
    blinking = false;
    timer.stop();
    set(false);
}

template <I_GPIO_OUT GPIO, I_OneShotTimer Timer>
void Light<GPIO, Timer>::blink(light::pattern p) noexcept {
    auto lock = std::lock_guard{m};
    timer.stop();
    blinking = true;
    current = p;
    set(true);
    timer.start(p.on);
}

// Timer callback. on()/off() may have run since this tick was scheduled,
// hence the blinking check under the lock.
template <I_GPIO_OUT GPIO, I_OneShotTimer Timer>
void Light<GPIO, Timer>::tick() noexcept {
    auto lock = std::lock_guard{m};
    if (!blinking)
        return;
    set(!lit.load());
    timer.start(lit.load() ? current.on : current.off);
}

template <I_GPIO_OUT GPIO, I_OneShotTimer Timer>
void Light<GPIO, Timer>::set(bool new_lit) noexcept {
    gpio.set_level(new_lit ? gpio::level::high : gpio::level::low);
    lit.store(new_lit);
}
