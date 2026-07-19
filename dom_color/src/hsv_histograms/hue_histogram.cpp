#include "hue_histogram.hpp"
#include "color_aux.hpp"
#include <cmath>

namespace hue_hist {
Dominant Dominant::grey =
    Dominant{.bounds = {.lower = 0, .upper = 0}, .count = 0, .is_grey = true};
}

HueHistogram::HueHistogram(uint8_t alpha, uint8_t beta)
    : histogram{HSV::kMaxHue},
      sumator{histogram.get_clusters(), histogram.get_upper_bound()},
      analyzer{histogram, sumator}, alpha{alpha}, beta{beta} {}

hue_hist::Dominant HueHistogram::get_dominant() {
    auto dominant_hue = hue_hist::Dominant{};
    if (alpha * grey_count >
        get_count()) { // default threshhold is 1/129 of non-grey fully
                       // saturated and valued pixels
        dominant_hue = hue_hist::Dominant::grey;
        dominant_hue.count = grey_count;
    } else {
        analyzer.set_alpha(calculate_beta());
        auto dominant_value = analyzer.get_dominant();
        if (dominant_value.count == 0) {
            dominant_hue = hue_hist::Dominant::grey;
            dominant_hue.count = grey_count;
        } else {
            dominant_hue = hue_hist::Dominant{
                .bounds = {.lower = dominant_value.bounds.lower,
                           .upper = dominant_value.bounds.upper},
                .hue = dominant_value.value,
                .count = dominant_value.count,
                .is_grey = false};
        }
    }
    return dominant_hue;
}

uint16_t HueHistogram::add_pixel(const HSV &hsv) {
    uint16_t hue_index = 0;
    if (color::is_grey(hsv)) {
        hue_index = UINT16_MAX;
        ++grey_count;
    } else {
        uint16_t temp_sv = hsv.s * hsv.v >> 15;
        temp_sv = std::min<uint16_t>(temp_sv, 0x7FFF);
        uint8_t weight = temp_sv >> 7;
        histogram.add(hsv.h, weight);
    }
    return hue_index;
}

void HueHistogram::reset() {
    histogram.reset();
    grey_count = 0;
}

uint8_t HueHistogram::calculate_beta() const {
    return std::min<uint32_t>(alpha * beta * grey_count / get_count(), 255);
}
