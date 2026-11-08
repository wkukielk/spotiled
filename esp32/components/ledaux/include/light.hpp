#ifndef LEDAUX_LIGHT_HPP
#define LEDAUX_LIGHT_HPP

#include "I_GPIO.hpp"
#include "I_Timer.hpp"
#include <atomic>
#include <chrono>
#include <mutex>

namespace light {
struct pattern {
    std::chrono::milliseconds on;
    std::chrono::milliseconds off;
};
} // namespace light

template <I_GPIO_OUT GPIO, I_OneShotTimer Timer> class Light {
  public:
    explicit Light(gpio_num_t pin);
    void on() noexcept;
    void off() noexcept;
    void blink(light::pattern p) noexcept;
    bool is_on() const noexcept { return lit.load(); }

  private:
    void tick() noexcept;
    void set(bool new_lit) noexcept;

    GPIO gpio;
    std::atomic<bool> lit = false;
    std::mutex m;
    bool blinking = false;
    light::pattern current{};
    Timer timer;
};

#include "light.tpp"

#endif
