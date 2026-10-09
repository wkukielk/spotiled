#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>

#include "dominant_color.hpp"
#include "hue_histogram.hpp"
#include "rgb.hpp"
#include "sv_histogram.hpp"

namespace
{
    using ExampleHueHistogram = GreyFilterHueHistogram<WCAGFilterHueHistogram<HueHistogram>>;

    std::ostream &operator<<(std::ostream &os, const RGB &c)
    {
        return os << "RGB(" << int(c.r) << ", " << int(c.g) << ", " << int(c.b) << ")";
    }
} // namespace

int main()
{
    // 64x64 RGB565 image: mostly red with a block of blue.
    std::vector<uint16_t> pixels(64 * 64, RGB(200, 30, 30).to_pixel16());
    std::fill(pixels.begin(), pixels.begin() + 1000, RGB(30, 30, 200).to_pixel16());

    std::cout << "default: " << get_dominating_color(pixels) << '\n';

    auto rgb = RGB{0, 6, 10};
    std::cout << color::is_grey(HSV{rgb}) << '\n';
}
