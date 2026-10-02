#include <gtest/gtest.h>

#include "dominant_color.hpp"
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
        Case{"noisy_orange", noisy_orange(), {225, 66, 11}}),
    [](const testing::TestParamInfo<Case> &info) { return info.param.name; });
