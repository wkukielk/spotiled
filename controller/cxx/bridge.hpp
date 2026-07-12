#ifndef LED_MANAGER_HPP
#define LED_MANAGER_HPP

#include "dominant_color.hpp"
#include "rust/cxx.h"

RGB get_dominating_color(rust::Slice<const uint16_t> pixels);

#endif
