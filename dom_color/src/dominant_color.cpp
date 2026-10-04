#include "dominant_color.hpp"
#include "color_aux.hpp"
#include "hue_histogram.hpp"
#include "sv_histogram.hpp"
#include "v_cluster.hpp"
#include <algorithm>
#include <ranges>

using namespace std;

namespace {
hue_hist::Dominant avg_hue(std::span<const uint16_t> bmp,
                           GreyFilterHueHistogram<HueHistogram> &hue_histogram) {
    hue_histogram.reset();
    ranges::for_each(bmp, [&](auto px) { hue_histogram.add_pixel(HSV{px}); });
    return hue_histogram.get_dominant();
}

sv_hist::Dominant avg_sv(std::span<const uint16_t> bmp, hist::Bounds bounds,
                         SVHistogram &sv_histogram) {
    sv_histogram.reset();
    for (auto px : bmp) {
        auto hsv = HSV{px};
        if (color::in_hue_interval(bounds.lower, bounds.upper, hsv.h))
            sv_histogram.add_pixel(hsv);
    }
    return sv_histogram.get_dominant();
}

DominantV avg_grey(std::span<const uint16_t> bmp) {
    VCluster v_cluster{};
    for (auto px : bmp) {
        auto hsv = HSV{px};
        if (color::is_grey(hsv))
            v_cluster.add(hsv);
    }
    return DominantV{.v = v_cluster.get_representant(),
                     .count = v_cluster.get_count()};
}

HSV avg_hsv(std::span<const uint16_t> bmp,
            GreyFilterHueHistogram<HueHistogram> &hue_histogram,
            SVHistogram &sv_histogram) {
    auto dominant_hue = avg_hue(bmp, hue_histogram);
    sv_hist::Dominant dominant_sv{};
    if (dominant_hue.is_grey)
        dominant_sv.value.v = avg_grey(bmp).v;
    else
        dominant_sv = avg_sv(bmp, dominant_hue.bounds, sv_histogram);
    return HSV{dominant_hue.hue, dominant_sv.value.s, dominant_sv.value.v};
}

} // namespace

RGB get_dominating_color(std::span<const uint16_t> pixels) {
    GreyFilterHueHistogram<HueHistogram> hue_histogram{};
    SVHistogram sv_histogram{};
    return get_dominating_color(pixels, hue_histogram, sv_histogram);
}

RGB get_dominating_color(std::span<const uint16_t> pixels,
                         GreyFilterHueHistogram<HueHistogram> &hue_histogram,
                         SVHistogram &sv_histogram) {
    return RGB{avg_hsv(pixels, hue_histogram, sv_histogram)};
}
