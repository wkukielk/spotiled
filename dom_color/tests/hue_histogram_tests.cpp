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
    histogram.add_pixel(
        HSV{deg(120), (uint16_t)(HSV::kMaxSat / 2), HSV::kMaxVal});
    EXPECT_EQ(histogram.get_count(), 128u);
}

TEST(HueHistogram, DominantOfSingleHue) {
    HueHistogram histogram;
    for (int i = 0; i < 100; ++i)
        histogram.add_pixel(HSV{deg(200), HSV::kMaxSat, HSV::kMaxVal});
    auto dominant = histogram.get_dominant();
    EXPECT_FALSE(dominant.is_grey);
    EXPECT_NEAR(dominant.hue, deg(200), deg(1));
}

TEST(HueHistogram, EmptyIsGrey) {
    HueHistogram histogram;
    auto dominant = histogram.get_dominant();
    EXPECT_TRUE(dominant.is_grey);
    EXPECT_EQ(dominant.count, 0u);
}

class GreyFilterHistogram : public testing::Test {
  protected:
    GreyFilterHueHistogram<HueHistogram> histogram{[] { return HueHistogram{}; },
                                                   3};
};

TEST_F(GreyFilterHistogram, MostlyGreyIsGrey) {
    for (int i = 0; i < 100; ++i)
        histogram.add_pixel(HSV{RGB{128, 128, 128}});
    histogram.add_pixel(HSV{deg(200), HSV::kMaxSat, HSV::kMaxVal});
    EXPECT_TRUE(histogram.get_dominant().is_grey);
}

TEST_F(GreyFilterHistogram, EmptyIsGrey) {
    EXPECT_TRUE(histogram.get_dominant().is_grey);
}

TEST_F(GreyFilterHistogram, GreyPixelsAreCountedSeparately) {
    histogram.add_pixel(HSV{RGB{128, 128, 128}});
    EXPECT_EQ(histogram.get_count(), 0u);
    EXPECT_EQ(histogram.get_grey_count(), 3u);
}

TEST_F(GreyFilterHistogram, ResetClearsCounts) {
    histogram.add_pixel(HSV{deg(200), HSV::kMaxSat, HSV::kMaxVal});
    histogram.add_pixel(HSV{RGB{128, 128, 128}});
    histogram.reset();
    EXPECT_EQ(histogram.get_count(), 0u);
    EXPECT_EQ(histogram.get_grey_count(), 0u);
}

class WCAGFilterHistogram : public testing::Test {
  protected:
    WCAGFilterHueHistogram<HueHistogram> histogram{[] { return HueHistogram{}; },
                                                   45};
};

TEST_F(WCAGFilterHistogram, KeepsDarkColor) {
    histogram.add_pixel(HSV{RGB{0, 0, 128}}); // contrast ~16:1
    EXPECT_GT(histogram.get_count(), 0u);
}

TEST_F(WCAGFilterHistogram, DropsLightColor) {
    histogram.add_pixel(HSV{RGB{255, 255, 0}}); // contrast ~1.07:1
    EXPECT_EQ(histogram.get_count(), 0u);
    EXPECT_TRUE(histogram.get_dominant().is_grey);
}

TEST_F(WCAGFilterHistogram, ThresholdFollowsGamma) {
    auto red = HSV{RGB{255, 0, 0}}; // contrast ~4.0:1
    histogram.add_pixel(red);
    EXPECT_EQ(histogram.get_count(), 0u);
    histogram.set_gamma(35);
    histogram.add_pixel(red);
    EXPECT_GT(histogram.get_count(), 0u);
}

TEST_F(WCAGFilterHistogram, GammaZeroKeepsEverything) {
    histogram.set_gamma(0);
    histogram.add_pixel(HSV{RGB{255, 255, 0}});
    EXPECT_GT(histogram.get_count(), 0u);
}

TEST(WhiteTextContrast, WhiteBackgroundIsExactlyOne) {
    EXPECT_TRUE(color::white_text_contrast_at_least(RGB{255, 255, 255}, 10));
    EXPECT_FALSE(color::white_text_contrast_at_least(RGB{255, 255, 255}, 11));
}

TEST(WhiteTextContrast, BlackBackgroundIsExactlyTwentyOne) {
    EXPECT_TRUE(color::white_text_contrast_at_least(RGB{0, 0, 0}, 210));
    EXPECT_FALSE(color::white_text_contrast_at_least(RGB{0, 0, 0}, 211));
}
