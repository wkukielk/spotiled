#ifndef LED_MANAGER_HPP
#define LED_MANAGER_HPP

#include "rgb.hpp"
#include "rust/cxx.h"

RGB led_manager_get_color(rust::Slice<const uint16_t> pixels);

#endif
