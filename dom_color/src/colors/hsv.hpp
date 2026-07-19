#ifndef HSV_HPP
#define HSV_HPP

#include <cstdint>

struct RGB;

struct floatHSV
{
    float h;
    float s;
    float v;
};

struct SV
{
    uint16_t s;
    uint16_t v;
};

struct HSV
{
    uint16_t h; // 0000 0000 0.000..
    uint16_t s; // 0.000..
    uint16_t v; // 0.000..

    HSV() = default;
    HSV(uint16_t h, uint16_t s, uint16_t v);
    HSV(float h, float s, float v);
    HSV(uint16_t pixel);
    HSV(uint32_t pixel);
    HSV(const RGB &);
    uint16_t to_pixel16() const;
    uint32_t to_pixel32() const;
    floatHSV to_float() const;
    bool operator==(const HSV &) const;

    static const uint16_t kMaxHue = (360 << 7);
    static const uint16_t kMaxSat = (1 << 15);
    static const uint16_t kMaxVal = (1 << 15);
};

#endif
