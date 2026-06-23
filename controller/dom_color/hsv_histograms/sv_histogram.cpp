#include "sv_histogram.hpp"
#include "color_aux.hpp"
#include <cstring>

// const uint8_t SVHistogram::weights[sz][sz] = {
//     1, 1, 1, 2, 2, 2, 2, 2, 3,  3,  3,  3,
//     1, 2, 2, 2, 3, 3, 3, 4, 4,  4,  4,  4,
//     1, 2, 3, 3, 3, 4, 4, 4, 5,  5,  5,  6,
//     2, 2, 3, 4, 4, 4, 5, 5, 6,  6,  6,  6,
//     2, 3, 3, 4, 5, 5, 5, 6, 6,  7,  7,  7,
//     2, 3, 4, 4, 5, 6, 6, 6, 7,  7,  8,  8,
//     2, 3, 4, 5, 5, 6, 7, 7, 7,  8,  8,  9,
//     2, 4, 4, 5, 6, 6, 7, 8, 8,  8,  9,  9,
//     3, 4, 5, 6, 6, 7, 7, 8, 9,  9,  9,  10,
//     3, 4, 5, 6, 7, 7, 8, 8, 9,  10, 10, 10,
//     3, 4, 5, 6, 7, 8, 8, 9, 9,  10, 11, 11,
//     3, 4, 6, 6, 7, 8, 9, 9, 10, 10, 11, 12,
// };

const uint8_t SVHistogram::weights[hist2D::sz][hist2D::sz] = {
    {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12},
    {2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24},
    {3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36},
    {4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48},
    {5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60},
    {6, 12, 18, 24, 30, 36, 42, 48, 54, 60, 66, 72},
    {7, 14, 21, 28, 35, 42, 49, 56, 63, 70, 77, 84},
    {8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96},
    {9, 18, 27, 36, 45, 54, 63, 72, 81, 90, 99, 108},
    {10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120},
    {11, 22, 33, 44, 55, 66, 77, 88, 99, 110, 121, 132},
    {12, 24, 36, 48, 60, 72, 84, 96, 108, 120, 132, 144}};

SVHistogram::SVHistogram()
    : histogram{HSV::kMaxSat + 1, HSV::kMaxVal + 1}, sumator{histogram},
      analyzer{histogram, sumator} {}

sv_hist::Index SVHistogram::add_pixel(const HSV &hsv) {
    uint8_t s_bit = hsv.s >> 15;
    uint8_t v_bit = hsv.v >> 15;
    auto index =
        sv_hist::Index{.s = (uint8_t)(((hsv.s - s_bit) * hist2D::sz) >> 15),
                       .v = (uint8_t)(((hsv.v - v_bit) * hist2D::sz) >> 15)};
    histogram.add(Value{.x = hsv.s, .y = hsv.v}, weights[index.s][index.v]);
    return index;
}

void SVHistogram::add_bpp32(const uint32_t *bitmap, size_t length) {
    for (auto bitmap_end = bitmap + length; bitmap != bitmap_end; ++bitmap)
        add_pixel(HSV{*bitmap});
}

sv_hist::Dominant SVHistogram::get_dominant() const {
    auto dominant_value = analyzer.get_dominant();
    return {.value = sv_hist::SV{.s = dominant_value.value.x,
                                 .v = dominant_value.value.y},
            .bounds = sv_hist::SVBounds{
                .s = hist2D::Bounds{.lower = dominant_value.bounds.x.lower,
                                    .upper = dominant_value.bounds.x.upper},
                .v = hist2D::Bounds{.lower = dominant_value.bounds.y.lower,
                                    .upper = dominant_value.bounds.y.upper}}};
}
