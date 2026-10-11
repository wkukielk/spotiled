#include "dominant_color.hpp"
#include "color_aux.hpp"
#include "hue_histogram.hpp"
#include "sv_histogram.hpp"
#include "v_cluster.hpp"

using namespace std;

namespace {
sv_hist::Dominant avg_sv(std::span<const uint16_t> bmp, hist::Bounds bounds,
                         SVHistogram &sv_histogram) {
    sv_histogram.reset();
    for (auto px : bmp) {
        auto hsv = HSV{px};
        auto in_ivl = color::in_hue_interval(bounds.lower, bounds.upper, hsv.h);
        if (in_ivl && !color::is_grey(hsv))
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

} // namespace

RGB detail::from_dominant_hue(std::span<const uint16_t> pixels,
                              const hue_hist::Dominant &dominant_hue,
                              SVHistogram &sv_histogram) {
    sv_hist::Dominant dominant_sv{};
    if (dominant_hue.is_grey)
        dominant_sv.value.v = avg_grey(pixels).v;
    else
        dominant_sv = avg_sv(pixels, dominant_hue.bounds, sv_histogram);
    return RGB{HSV{dominant_hue.hue, dominant_sv.value.s, dominant_sv.value.v}};
}

RGB get_dominating_color(std::span<const uint16_t> pixels) {
    using WCAGHist = WCAGFilterHueHistogram<HueHistogram>;
    using Hist = GreyFilterHueHistogram<WCAGHist>;
    uint8_t alpha = 2;
    uint8_t beta = 59;
    uint8_t gamma = 25;

    auto hue_histogram = Hist{WCAGHist{gamma}, alpha, beta};
    SVHistogram sv_histogram{};
    return get_dominating_color(pixels, hue_histogram, sv_histogram);
}
