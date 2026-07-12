#ifndef RGB_PP
#define RGB_PP

#include <cstdint>

struct HSV;

struct RGB {
    uint8_t r;
    uint8_t g;
    uint8_t b;

    RGB() = default;
    RGB(uint8_t r, uint8_t g, uint8_t b);
    RGB(uint16_t pixel);
    RGB(uint32_t pixel);
    RGB(const HSV &);
    uint16_t to_pixel16() const;
    uint32_t to_pixel32() const;
    bool operator==(const RGB &) const;
};

#endif
