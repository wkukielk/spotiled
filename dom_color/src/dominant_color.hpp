#ifndef DOMINANT_COLOR_HPP
#define DOMINANT_COLOR_HPP

#include "hue_histogram.hpp"
#include "rgb.hpp"
#include "sv_histogram.hpp"
#include <cstdint>
#include <span>

RGB get_dominating_color(std::span<const uint16_t> pixels);
// Reuses the given histograms (and their parameters); resets them first.
template <I_HueHistogram H>
RGB get_dominating_color(std::span<const uint16_t> pixels, H &hue_histogram,
                         SVHistogram &sv_histogram);

namespace detail {
RGB from_dominant_hue(std::span<const uint16_t> pixels,
                      const hue_hist::Dominant &dominant_hue,
                      SVHistogram &sv_histogram);
} // namespace detail

#include "dominant_color.tpp"

#endif
