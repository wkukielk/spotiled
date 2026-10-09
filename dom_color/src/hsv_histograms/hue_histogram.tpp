#include "color_aux.hpp"

template <I_HueHistogram H>
GreyFilterHueHistogram<H>::GreyFilterHueHistogram(uint8_t alpha, uint8_t beta)
    requires std::default_initializable<H>
    : alpha{alpha}, beta{beta} {}

template <I_HueHistogram H>
GreyFilterHueHistogram<H>::GreyFilterHueHistogram(H &&hue_histogram,
                                                  uint8_t alpha, uint8_t beta)
    : hue_histogram{std::move(hue_histogram)}, alpha{alpha}, beta{beta} {}

template <I_HueHistogram H>
hue_hist::Dominant GreyFilterHueHistogram<H>::get_dominant() {
    if (get_count() == 0 || alpha * grey_count > get_count())
        return hue_hist::create_grey(grey_count);
    else {
        hue_histogram.set_alpha(calculate_beta());
        return hue_histogram.get_dominant();
    }
}

template <I_HueHistogram H>
void GreyFilterHueHistogram<H>::add_pixel(const HSV &hsv) {
    if (color::is_grey(hsv))
        ++grey_count;
    else
        hue_histogram.add_pixel(hsv);
}

template <I_HueHistogram H> void GreyFilterHueHistogram<H>::reset() {
    grey_count = 0;
    hue_histogram.reset();
}

template <I_HueHistogram H>
uint8_t GreyFilterHueHistogram<H>::calculate_beta() const {
    uint64_t grey = (uint64_t)alpha * grey_count;
    return std::min<unsigned>(beta + grey / get_count(), 255u);
}

template <I_HueHistogram H>
WCAGFilterHueHistogram<H>::WCAGFilterHueHistogram(uint8_t gamma)
    requires std::default_initializable<H>
    : gamma{gamma} {}

template <I_HueHistogram H>
WCAGFilterHueHistogram<H>::WCAGFilterHueHistogram(H &&hue_histogram,
                                                  uint8_t gamma)
    : hue_histogram{std::move(hue_histogram)}, gamma{gamma} {}

template <I_HueHistogram H>
void WCAGFilterHueHistogram<H>::add_pixel(const HSV &hsv) {
    if (color::white_text_contrast_at_least(RGB{hsv}, gamma))
        hue_histogram.add_pixel(hsv);
}
