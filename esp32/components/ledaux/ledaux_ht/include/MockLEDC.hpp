#ifndef LEDAUX_HT_MOCK_LEDC_HPP
#define LEDAUX_HT_MOCK_LEDC_HPP

#include "ledaux.hpp"
#include "rgb.hpp"
#include <gmock/gmock.h>

class MockLEDC {
  public:
    MOCK_METHOD(esp_err_t, fade_to, (RGB rgb, uint32_t duration_ms),
               (noexcept));
    MOCK_METHOD(esp_err_t, set_color, (RGB rgb), (noexcept));
    MOCK_METHOD(RGB, get_current_color, (), (const, noexcept));
};

static_assert(I_LEDC<MockLEDC>);

#endif
