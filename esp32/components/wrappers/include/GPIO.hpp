#ifndef LEDAUX_GPIO_HPP
#define LEDAUX_GPIO_HPP

#include "I_GPIO.hpp"

class GPIO_IN {
  public:
    GPIO_IN(gpio_num_t pin, gpio::pull pull);
    ~GPIO_IN() noexcept;
    gpio::level get_level() const noexcept;

  private:
    gpio_num_t pin;
};

class GPIO_OUT {
  public:
    GPIO_OUT(gpio_num_t pin);
    ~GPIO_OUT() noexcept;
    void set_level(gpio::level level);

  private:
    gpio_num_t pin;
};

static_assert(I_GPIO_IN<GPIO_IN>);
static_assert(I_GPIO_OUT<GPIO_OUT>);

#endif
