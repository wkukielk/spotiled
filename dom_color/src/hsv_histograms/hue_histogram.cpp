#include "hue_histogram.hpp"
#include "color_aux.hpp"
#include <cmath>

using namespace std;

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

HueHistogram::HueHistogram()
    : histogram{std::make_unique<hist::Histogram>(HSV::kMaxHue)},
      analyzer{*histogram} {}

hue_hist::Dominant HueHistogram::get_dominant() const {
    if (get_count() == 0)
        return hue_hist::create_grey(0);
    else {
        auto dominant_value = analyzer.get_dominant();
        if (dominant_value.count == 0)
            return hue_hist::create_grey(0);
        else
            return hue_hist::convert_dominant(dominant_value);
    }
}

void HueHistogram::set_alpha(uint8_t alpha) { analyzer.set_alpha(alpha); }

void HueHistogram::add_pixel(const HSV &hsv) {
    uint16_t temp_sv = hsv.s * hsv.v >> 15;
    temp_sv = std::min<uint16_t>(temp_sv, 0x7FFF);
    uint8_t weight = temp_sv >> 7;
    histogram->add(hsv.h, weight);
}

void HueHistogram::reset() { histogram->reset(); }
