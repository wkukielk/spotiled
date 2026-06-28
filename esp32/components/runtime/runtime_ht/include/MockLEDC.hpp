#ifndef MOCK_LEDC_HPP
#define MOCK_LEDC_HPP

#include "ledaux.hpp"
#include "rgb.hpp"
#include <gmock/gmock.h>

class MockLEDC {
  public:
    MOCK_METHOD(esp_err_t, set_color, (RGB rgb), (noexcept));
    MOCK_METHOD(RGB, get_current_color, (), (const, noexcept));
};

static_assert(I_LEDC<MockLEDC>);

#endif
