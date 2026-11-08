#ifndef LEDAUX_LIGHT_HPP
#define LEDAUX_LIGHT_HPP

#include "I_GPIO.hpp"

template <I_GPIO_OUT GPIO> class Light {
  public:
    explicit Light(gpio_num_t pin);
    void on();
    void off();
    void toggle();
    bool is_on() const noexcept { return lit; }

  private:
    void set(bool new_lit);

    GPIO gpio;
    bool lit = false;
};

#include "light.tpp"

#endif