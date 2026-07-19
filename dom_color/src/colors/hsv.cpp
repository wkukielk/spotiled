#include "hsv.hpp"
#include "rgb.hpp"
#include <algorithm>


HSV::HSV(uint16_t h, uint16_t s, uint16_t v) : h{h}, s{s}, v{v} {}

HSV::HSV(float h, float s, float v) :
    h{ (uint16_t)(h * (1 << 7)) },
    s{ (uint16_t)(s * (1 << 15)) },
    v{ (uint16_t)(v * (1 << 15)) } {}

HSV::HSV(uint16_t pixel) {
    auto rgb = RGB{pixel};
    *this = HSV{rgb};
}

HSV::HSV(uint32_t pixel) {
    auto rgb = RGB{pixel};
    *this = HSV{rgb};
}

HSV::HSV(const RGB& rgb) {
    int32_t r = rgb.r << 23;
    int32_t g = rgb.g << 23;
    int32_t b = rgb.b << 23;
    auto cmax = std::max(std::max(r, g), b);
    auto cmin = std::min(std::min(r, g), b);
    int32_t delta = (cmax - cmin) >> 23;
    int32_t hue = 0;

    if(delta != 0) {
        if(cmax == r) {
            hue = (g - b) / delta;
            if(hue < 0) {
                hue += 6 << 23;
            }
        } else if(cmax == g)
            hue = (b - r) / delta + (2 << 23);
        else if(cmax == b)
            hue = (r - g) / delta + (4 << 23);
        hue *= 60;
        h = (hue + (1 << 15))  >> 16;   // won't be >= 360 deg; (0xFF0001)

        cmax >>= 23;
        if(cmax != 0) {
            delta <<= 15;
            s = (delta + cmax / 2) / cmax;  // delta <= cmax, so s <= 1 either
        } else
            s = 0;

        cmax <<= 15;
        v = (cmax + 127) / 255;     // cmax <= 255 << 15, v <= 1
    } else {
        h = 0;
        s = 0;
        v = ( (cmax + 127) / 255 ) >> 8;
    }
}

uint16_t HSV::to_pixel16() const {
    auto rgb = RGB{*this};
    return rgb.to_pixel16();
}

uint32_t HSV::to_pixel32() const {
    auto rgb = RGB{*this};
    return rgb.to_pixel32();
}

floatHSV HSV::to_float() const {
    return floatHSV{
        .h = h / float(1 << 7),
        .s = s / float(1 << 15),
        .v = v / float(1 << 15)
    };
}

bool HSV::operator==(const HSV& other) const {
    return
        h >> 7 == other.h >> 7 &&
        s >> 7 == other.s >> 7 &&
        v >> 7 == other.v >> 7;
}
