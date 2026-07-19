#ifndef DOMINANT_COLOR_HPP
#define DOMINANT_COLOR_HPP

#include "rgb.hpp"
#include <cstdint>
#include <span>

RGB get_dominating_color(std::span<const uint16_t> pixels);

#endif
