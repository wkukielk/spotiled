#ifndef SERVER_LED_COLOR_HANDLER_HPP
#define SERVER_LED_COLOR_HANDLER_HPP

#include "ledaux.hpp"
#include "rgb.hpp"
#include <optional>

template <I_LEDC LEDC> class LedColorHandler {
  public:
    explicit LedColorHandler(LEDC &ledc) : ledc{ledc} {};
    esp_err_t process(std::optional<RGB> color);

  protected:
    LEDC &ledc;
};

template <typename T>
concept I_LedColorHandler = requires(T t, std::optional<RGB> color) {
    { t.process(color) } -> std::same_as<esp_err_t>;
};

#include "led_color_handler.tpp"

#endif
