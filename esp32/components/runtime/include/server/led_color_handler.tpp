#ifndef SERVER_LED_COLOR_HANDLER_HPP
#include "server/led_color_handler.hpp"
#endif

template <I_LEDC LEDC>
esp_err_t LedColorHandler<LEDC>::process(std::optional<RGB> color) {
    if (!color)
        return ESP_ERR_INVALID_ARG;
    return ledc.set_color(*color);
}
