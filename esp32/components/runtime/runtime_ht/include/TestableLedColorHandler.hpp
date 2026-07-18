#ifndef TESTABLE_LED_COLOR_HANDLER_HPP
#define TESTABLE_LED_COLOR_HANDLER_HPP

#include "MockLEDCHolder.hpp"
#include "server/led_color_handler.hpp"

class TestableLedColorHandler : private detail::MockLEDCHolder,
                                public LedColorHandler<MockLEDC> {
  public:
    TestableLedColorHandler() : LedColorHandler{mock_ledc} {}
    MockLEDC &get_ledc() { return mock_ledc; }
};

#endif
