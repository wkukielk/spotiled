#include "hue_histogram.hpp"
#include "color_aux.hpp"
#include <cmath>

namespace hue_hist {
Dominant Dominant::grey =
    Dominant{.bounds = {.lower = 0, .upper = 0}, .count = 0, .is_grey = true};

Dominant create_grey(uint32_t grey_count) {
    auto grey = Dominant::grey;
    grey.count = grey_count;
    return grey;
}

Dominant convert_dominant(const hist::Dominant &dominant) {
    return Dominant{.bounds = {.lower = dominant.bounds.lower,
                               .upper = dominant.bounds.upper},
                    .hue = dominant.value,
                    .count = dominant.count,
                    .is_grey = false};
}
} // namespace hue_hist

HueHistogram::HueHistogram(uint8_t alpha, uint8_t beta)
    : histogram{HSV::kMaxHue}, analyzer{histogram}, alpha{alpha}, beta{beta} {}

hue_hist::Dominant HueHistogram::get_dominant() {
    if (get_count() == 0 || alpha * grey_count > get_count())
        return hue_hist::create_grey(grey_count);
    else {
        analyzer.set_alpha(calculate_beta());
        auto dominant_value = analyzer.get_dominant();
        if (dominant_value.count == 0)
            return hue_hist::create_grey(grey_count);
        else
            return hue_hist::convert_dominant(dominant_value);
    }
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
