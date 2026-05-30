#include "MockLED.hpp"
#include "ledaux.hpp"

class TestableLEDC : public LEDC<MockLED> {
  public:
    using LEDC::LEDC;
    MockLED &get_led() { return led_; };
};
