#ifndef COLOR_AUX_HPP
#define COLOR_AUX_HPP

#include "hsv.hpp"

namespace color {
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
