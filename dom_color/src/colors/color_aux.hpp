#ifndef COLOR_AUX_HPP
#define COLOR_AUX_HPP

#include "hsv.hpp"
#include "rgb.hpp"
#include <array>
#include <cmath>

namespace color {
// sRGB channel -> linear light, 0..65535.
inline uint16_t srgb_to_linear(uint8_t channel) {
    static const auto table = [] {
        std::array<uint16_t, 256> t{};
        for (int i = 0; i < 256; ++i) {
            double c = i / 255.0;
            double linear =
                c <= 0.04045 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4);
            t[i] = (uint16_t)std::lround(linear * 65535);
        }
        return t;
    }();
    return table[channel];
}

// WCAG 2 relative luminance, 0 (black) .. 65535 (white).
inline uint32_t relative_luminance(const RGB &rgb) {
    // 0.2126, 0.7152, 0.0722 in Q16; they sum to 65536.
    return (13933u * srgb_to_linear(rgb.r) + 46871u * srgb_to_linear(rgb.g) +
            4732u * srgb_to_linear(rgb.b)) >>
           16;
}

// WCAG contrast of white text on rgb, 1.05 / (L + 0.05), is at least
// min_contrast / 10. Rearranged to min_contrast * (20 L + 1) <= 210.
inline bool white_text_contrast_at_least(const RGB &rgb, uint8_t min_contrast) {
    return min_contrast * (20 * relative_luminance(rgb) + 65535) <= 210u * 65535;
}

inline bool is_grey(const HSV &hsv) {
    uint8_t s = hsv.s >> 8; // 0.10 == 0x0C
    uint8_t v = hsv.v >> 8; // 0.05 == 0x06
    uint16_t sv = s * v;    // 0.03 == 0x01EB
    return s < 0x0C || v < 0x06 || sv < 0x01EB;
}

inline uint16_t hue_distance(uint16_t from, uint16_t to) {
    return to >= from ? to - from : (360 << 7) + to - from;
}

inline bool in_hue_interval(uint16_t lower_end, uint16_t upper_end,
                            uint16_t hue) {
    return hue_distance(lower_end, hue) <= hue_distance(lower_end, upper_end);
}
} // namespace color

#endif
