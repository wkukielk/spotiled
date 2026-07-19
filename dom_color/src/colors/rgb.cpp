#include "rgb.hpp"
#include "hsv.hpp"
#include <cmath>


RGB::RGB(uint16_t pixel) {
    auto blue = (uint8_t)( pixel << 3 );
    auto green = (uint8_t)( (pixel >> 3) & 0xFC );
    auto red = (uint8_t)( (pixel >> 8) & 0xF8 );
    r = red, g = green, b = blue;
}

RGB::RGB(uint32_t pixel) {
    auto blue = (uint8_t)pixel;
    auto green = (uint8_t)( (pixel >> 8) & 0xFF );
    auto red = (uint8_t)( (pixel >> 16) & 0xFF );
    r = red, g = green, b = blue;
}

RGB::RGB(uint8_t r, uint8_t g, uint8_t b) : r{r}, g{g}, b{b} {}

RGB::RGB(const HSV& hsv) {
    auto [h, s, v] = hsv;
    int32_t c = s * v  >> 15;
    int32_t x = c * ( (1 << 7) - std::abs((h / 60) % (2 << 7) - (1 << 7)) ) * 255;
    x >>= 7;
    int32_t m = (v - c) * 255;
    c *= 255;
    h >>= 7;
    int32_t r = 0;
    int32_t g = 0;
    int32_t b = 0;
    if (h < 60) {
        r = c;
        g = x;
    } else if (h < 120) {
        r = x;
        g = c;
    } else if (h < 180) {
        g = c;
        b = x;
    } else if (h < 240) {
        g = x;
        b = c;
    } else if (h < 300) {
        r = x;
        b = c;
    } else {
        r = c;
        b = x;
    }
    // max(rbg) <= 255 << 15
    this -> r = ( r + m + (1 << 14) ) >> 15;
    this -> g = ( g + m + (1 << 14) ) >> 15;
    this -> b = ( b + m + (1 << 14) ) >> 15;
}

uint16_t RGB::to_pixel16() const {
    uint8_t r5 = r;
    uint8_t g6 = g;
    uint8_t b5 = b;
    (r5 += 0x4) >>= 3;
    (g6 += 0x2) >>= 2;
    (b5 += 0x4) >>= 3;
    return b5 | (g6 << 5) | (r5 << 11);
}

uint32_t RGB::to_pixel32() const {
    return b | (g << 8) | (r << 16);
}

bool RGB::operator==(const RGB& other) const {
    return
        r == other.r &&
        g == other.g &&
        b == other.b;
}
