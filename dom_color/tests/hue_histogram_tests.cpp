#include <gtest/gtest.h>

#include "hue_histogram.hpp"
#include "rgb.hpp"

namespace {
constexpr uint16_t deg(uint16_t degrees) { return degrees << 7; }
} // namespace

// s * v == 1.0 is 0x8000 in Q15; it must clamp to weight 255, not wrap to 0.
TEST(HueHistogram, FullySaturatedPixelGetsMaxWeight) {
    HueHistogram histogram;
    histogram.add_pixel(HSV{deg(120), HSV::kMaxSat, HSV::kMaxVal});
    EXPECT_EQ(histogram.get_count(), 255u);
}

TEST(HueHistogram, WeightScalesWithSaturationTimesValue) {
    HueHistogram histogram;
    histogram.add_pixel(HSV{deg(120), (uint16_t)(HSV::kMaxSat / 2), HSV::kMaxVal});
    EXPECT_EQ(histogram.get_count(), 128u);
}

TEST(HueHistogram, GreyPixelsAreCountedSeparately) {
    HueHistogram histogram;
    EXPECT_EQ(histogram.add_pixel(HSV{RGB{128, 128, 128}}), UINT16_MAX);
    EXPECT_EQ(histogram.get_count(), 0u);
    EXPECT_EQ(histogram.get_grey_count(), 3u); // default alpha == 3
}

TEST(HueHistogram, DominantOfSingleHue) {
    HueHistogram histogram;
    for (int i = 0; i < 100; ++i)
        histogram.add_pixel(HSV{deg(200), HSV::kMaxSat, HSV::kMaxVal});
    auto dominant = histogram.get_dominant();
    EXPECT_FALSE(dominant.is_grey);
    EXPECT_NEAR(dominant.hue, deg(200), deg(1));
}

TEST(HueHistogram, MostlyGreyIsGrey) {
    HueHistogram histogram;
    for (int i = 0; i < 100; ++i)
        histogram.add_pixel(HSV{RGB{128, 128, 128}});
    histogram.add_pixel(HSV{deg(200), HSV::kMaxSat, HSV::kMaxVal});
    EXPECT_TRUE(histogram.get_dominant().is_grey);
}

TEST(HueHistogram, ResetClearsCounts) {
    HueHistogram histogram;
    histogram.add_pixel(HSV{deg(200), HSV::kMaxSat, HSV::kMaxVal});
    histogram.add_pixel(HSV{RGB{128, 128, 128}});
    histogram.reset();
    EXPECT_EQ(histogram.get_count(), 0u);
    EXPECT_EQ(histogram.get_grey_count(), 0u);
}
