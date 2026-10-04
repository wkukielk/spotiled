#ifndef DOMINANT_COLOR_HPP
#define DOMINANT_COLOR_HPP

#include "hue_histogram.hpp"
#include "rgb.hpp"
#include "sv_histogram.hpp"
#include <cstdint>
#include <span>

RGB get_dominating_color(std::span<const uint16_t> pixels);
// Reuses the given histograms (and their parameters); resets them first.
RGB get_dominating_color(std::span<const uint16_t> pixels,
                         HueHistogram &hue_histogram,
                         SVHistogram &sv_histogram);

#endif
