#include <gtest/gtest.h>

#include "color_aux.hpp"
#include "hsv.hpp"
#include "rgb.hpp"

namespace {
uint64_t fnv1a(uint64_t hash, uint64_t value) {
    for (int i = 0; i < 8; ++i) {
        hash ^= (value >> (8 * i)) & 0xFF;
        hash *= 0x100000001b3ULL;
    }
    return hash;
}

constexpr uint64_t kFnvOffset = 0xcbf29ce484222325ULL;
constexpr uint16_t deg(uint16_t degrees) { return degrees << 7; }
} // namespace

TEST(RGB, DecodesPixel16) {
    EXPECT_EQ(RGB{(uint16_t)0xF800}, (RGB{248, 0, 0}));
    EXPECT_EQ(RGB{(uint16_t)0x07E0}, (RGB{0, 252, 0}));
    EXPECT_EQ(RGB{(uint16_t)0x001F}, (RGB{0, 0, 248}));
    EXPECT_EQ(RGB{(uint16_t)0xFFFF}, (RGB{248, 252, 248}));
}

TEST(RGB, DecodesPixel32) {
    EXPECT_EQ(RGB{(uint32_t)0x123456}, (RGB{0x12, 0x34, 0x56}));
}

TEST(RGB, Pixel16RoundTripIsExact) {
    for (uint32_t px = 0; px < 0x10000; ++px)
        ASSERT_EQ(RGB{(uint16_t)px}.to_pixel16(), px) << std::hex << px;
}

TEST(RGB, Pixel32RoundTripIsExact) {
    for (uint32_t px = 0; px < (1u << 24); px += 101)
        ASSERT_EQ(RGB{px}.to_pixel32(), px) << std::hex << px;
}

TEST(HSV, Primaries) {
    EXPECT_EQ(HSV{(RGB{255, 0, 0})}, (HSV{deg(0), HSV::kMaxSat, HSV::kMaxVal}));
    EXPECT_EQ(HSV{(RGB{0, 255, 0})},
              (HSV{deg(120), HSV::kMaxSat, HSV::kMaxVal}));
    EXPECT_EQ(HSV{(RGB{0, 0, 255})},
              (HSV{deg(240), HSV::kMaxSat, HSV::kMaxVal}));
}

TEST(HSV, GreysHaveZeroSaturation) {
    for (int c = 0; c < 256; ++c) {
        auto hsv = HSV{RGB{(uint8_t)c, (uint8_t)c, (uint8_t)c}};
        EXPECT_EQ(hsv.h, 0) << c;
        EXPECT_EQ(hsv.s, 0) << c;
    }
    EXPECT_EQ((HSV{RGB{255, 255, 255}}.v), uint16_t{HSV::kMaxVal});
    EXPECT_EQ((HSV{RGB{0, 0, 0}}.v), 0);
}

TEST(HSV, PrimariesConvertBackToRGB) {
    EXPECT_EQ(RGB{(HSV{deg(0), HSV::kMaxSat, HSV::kMaxVal})}, (RGB{255, 0, 0}));
    EXPECT_EQ(RGB{(HSV{deg(120), HSV::kMaxSat, HSV::kMaxVal})},
              (RGB{0, 255, 0}));
    EXPECT_EQ(RGB{(HSV{deg(240), HSV::kMaxSat, HSV::kMaxVal})},
              (RGB{0, 0, 255}));
}

// Golden-master hashes over every RGB565 pixel. They pin the exact fixed-point
// output of the current conversions; if one fails after a refactor, the
// numeric behaviour changed. Update the constant only if that was intended.
TEST(Golden, HSVFromEveryPixel16) {
    uint64_t hash = kFnvOffset;
    for (uint32_t px = 0; px < 0x10000; ++px) {
        HSV hsv{(uint16_t)px};
        hash = fnv1a(hash, (uint64_t)hsv.h << 32 | (uint64_t)hsv.s << 16 | hsv.v);
    }
    EXPECT_EQ(hash, 0xb33e2bb10d00c8d6ULL);
}

TEST(Golden, RGBFromHSVOfEveryPixel16) {
    uint64_t hash32 = kFnvOffset;
    uint64_t hash16 = kFnvOffset;
    for (uint32_t px = 0; px < 0x10000; ++px) {
        RGB rgb{HSV{(uint16_t)px}};
        hash32 = fnv1a(hash32, rgb.to_pixel32());
        hash16 = fnv1a(hash16, rgb.to_pixel16());
    }
    EXPECT_EQ(hash32, 0x9680fa8a7a4bdf13ULL);
    EXPECT_EQ(hash16, 0xd319314a9541e384ULL);
}

TEST(ColorAux, IsGrey) {
    EXPECT_TRUE(color::is_grey(HSV{RGB{128, 128, 128}}));
    EXPECT_TRUE(color::is_grey(HSV{RGB{0, 0, 0}}));
    EXPECT_TRUE(color::is_grey(HSV{RGB{8, 0, 0}})); // too dark
    EXPECT_FALSE(color::is_grey(HSV{RGB{255, 0, 0}}));
    EXPECT_FALSE(color::is_grey(HSV{RGB{200, 100, 50}}));
}

TEST(ColorAux, HueDistanceWrapsAround) {
    EXPECT_EQ(color::hue_distance(deg(10), deg(20)), deg(10));
    EXPECT_EQ(color::hue_distance(deg(350), deg(10)), deg(20));
    EXPECT_TRUE(color::in_hue_interval(deg(350), deg(10), deg(0)));
    EXPECT_FALSE(color::in_hue_interval(deg(350), deg(10), deg(180)));
}
