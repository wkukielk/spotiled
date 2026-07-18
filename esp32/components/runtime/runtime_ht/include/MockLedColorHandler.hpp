#ifndef MOCK_LED_COLOR_HANDLER_HPP
#define MOCK_LED_COLOR_HANDLER_HPP

#include "server/led_color_handler.hpp"
#include <gmock/gmock.h>

class MockLedColorHandler {
  public:
    MOCK_METHOD(esp_err_t, process, (std::optional<RGB> color), ());
};

static_assert(I_LedColorHandler<MockLedColorHandler>);

#endif
