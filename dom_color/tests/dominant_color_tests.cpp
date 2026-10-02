#include <gtest/gtest.h>

#include "dominant_color.hpp"
#include "hsv.hpp"
#include <random>
#include <vector>

namespace {
std::vector<uint16_t> solid(uint16_t px, size_t n = 256) {
    return std::vector<uint16_t>(n, px);
}

std::vector<uint16_t> concat(std::vector<uint16_t> a,
                             const std::vector<uint16_t> &b) {
    a.insert(a.end(), b.begin(), b.end());
    return a;
}

std::vector<uint16_t> random_image(unsigned seed) {
    std::mt19937 gen{seed};
    std::vector<uint16_t> image(64 * 64);
    for (auto &px : image)
        px = (uint16_t)gen();
    return image;
}

std::vector<uint16_t> noisy_orange() {
    std::mt19937 gen{42};
    std::vector<uint16_t> image(64 * 64);
    for (auto &px : image) {
        // Evaluate in a fixed order; argument evaluation order is unspecified.
        auto r = (uint8_t)(200 + gen() % 56);
        auto g = (uint8_t)(60 + gen() % 40);
        auto b = (uint8_t)(gen() % 40);
        px = RGB{r, g, b}.to_pixel16();
    }
    return image;
}

// Fully saturated pixels with hues spread evenly over centre +- 15 deg.
std::vector<uint16_t> hue_spread(int centre) {
    std::vector<uint16_t> image;
    for (int quarter = -60; quarter <= 60; ++quarter) {
        int hue = ((centre * 4 + quarter) % 1440 + 1440) % 1440;
        auto rgb = RGB{HSV{(uint16_t)(hue << 5), HSV::kMaxSat, HSV::kMaxVal}};
        auto px = (uint16_t)((rgb.r >> 3) << 11 | (rgb.g >> 2) << 5 | rgb.b >> 3);
        image.insert(image.end(), 8, px);
    }
    return image;
}

// Dominant window wraps: bright pixels just under 360 deg, dark ones at 0 deg,
// flanked by enough other hues that no single cluster wins on its own.
std::vector<uint16_t> wrapping_peak() {
    struct Part {
        uint16_t px;
        size_t n;
    };
    std::vector<uint16_t> image;
    for (auto [px, n] : {Part{0xF821, 40}, Part{0x7800, 80}, Part{0xF802, 30},
                         Part{0xF803, 30}, Part{0xF804, 30}, Part{0xF840, 30},
                         Part{0xF880, 30}, Part{0xF8C0, 30}})
        image.insert(image.end(), n, px);
    return image;
}

struct Case {
    const char *name;
    std::vector<uint16_t> pixels;
    RGB expected;
};

void PrintTo(const Case &c, std::ostream *os) { *os << c.name; }
} // namespace

void PrintTo(const RGB &c, std::ostream *os) {
    *os << "RGB{" << +c.r << ", " << +c.g << ", " << +c.b << "}";
}

class DominantColor : public testing::TestWithParam<Case> {};

TEST_P(DominantColor, MatchesGolden) {
    EXPECT_EQ(get_dominating_color(GetParam().pixels), GetParam().expected);
}

// Golden values captured from the implementation before refactoring.
INSTANTIATE_TEST_SUITE_P(
    Golden, DominantColor,
    testing::Values(
        Case{"red", solid(0xF800), {248, 0, 0}},
        Case{"green", solid(0x07E0), {0, 252, 0}},
        Case{"blue", solid(0x001F), {0, 0, 248}},
        Case{"yellow", solid(0xFFE0), {250, 252, 0}},
        Case{"magenta", solid(0xF81F), {248, 0, 248}},
        Case{"cyan", solid(0x07FF), {0, 252, 246}},
        Case{"white", solid(0xFFFF), {252, 252, 252}},
        Case{"black", solid(0x0000), {0, 0, 0}},
        Case{"grey", solid(0x8410), {128, 128, 128}},
        Case{"orange", solid(0xFD20), {248, 163, 0}},
        Case{"red300_blue100", concat(solid(0xF800, 300), solid(0x001F, 100)),
             {248, 0, 0}},
        Case{"green200_white200",
             concat(solid(0x07E0, 200), solid(0xFFFF, 200)), {0, 252, 0}},
        Case{"red50_black400", concat(solid(0xF800, 50), solid(0x0000, 400)),
             {248, 0, 0}},
        Case{"random_seed1", random_image(1), {183, 183, 183}},
        Case{"random_seed2", random_image(2), {171, 171, 171}},
        Case{"random_seed3", random_image(3), {185, 185, 185}},
        Case{"noisy_orange", noisy_orange(), {245, 71, 11}},
        Case{"hue_spread_120", hue_spread(120), {0, 252, 2}},
        Case{"hue_spread_0", hue_spread(0), {248, 2, 0}},
        Case{"hue_spread_355", hue_spread(355), {248, 2, 0}},
        Case{"hue_spread_5", hue_spread(5), {248, 2, 0}},
        Case{"wrapping_peak", wrapping_peak(), {120, 0, 2}}),
    [](const testing::TestParamInfo<Case> &info) { return info.param.name; });

TEST(DominantColorEdge, EmptyIsBlack) {
    EXPECT_EQ(get_dominating_color({}), (RGB{0, 0, 0}));
}
