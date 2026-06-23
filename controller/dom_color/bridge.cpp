#include "bridge.hpp"
#include "color_aux.hpp"
#include "hue_histogram.hpp"
#include "sv_histogram.hpp"
#include "v_cluster.hpp"
#include <span>
#include <algorithm>
#include <ranges>

using namespace std;

namespace
{
    hue_hist::Dominant avg_hue(std::span<const uint16_t> bmp)
    {
        HueHistogram hue_histogram{};
        ranges::for_each(bmp, [&](auto px)
                         { hue_histogram.add_pixel(HSV{px}); });
        return hue_histogram.get_dominant();
    }

    sv_hist::Dominant avg_sv(std::span<const uint16_t> bmp, hist::Bounds bounds)
    {
        SVHistogram sv_histogram{};
        for (auto px = bmp.begin(); px < bmp.end(); ++px)
        {
            auto hsv = HSV{*px};
            if (bounds.lower <= hsv.h && hsv.h <= bounds.upper)
                sv_histogram.add_pixel(hsv);
        }
        return sv_histogram.get_dominant();
    }

    DominantV avg_grey(std::span<const uint16_t> bmp)
    {
        VCluster v_cluster{};
        for (auto px = bmp.begin(); px < bmp.end(); ++px)
        {
            auto hsv = HSV{*px};
            if (color::is_grey(hsv))
                v_cluster.add(hsv);
        }
        return DominantV{.v = v_cluster.get_representant(),
                         .count = v_cluster.get_count()};
    }

    HSV avg_hsv(std::span<const uint16_t> bmp)
    {
        auto dominant_hue = avg_hue(bmp);
        sv_hist::Dominant dominant_sv{};
        if (dominant_hue.is_grey)
        {
            auto dominant_v = avg_grey(bmp);
            dominant_sv.value.v = dominant_v.v;
            dominant_sv.count = 0;
        }
        else
            dominant_sv = avg_sv(bmp, dominant_hue.bounds);
        return HSV{dominant_hue.hue, dominant_sv.value.s, dominant_sv.value.v};
    }

} // namespace

RGB get_dominating_color(rust::Slice<const uint16_t> pixels)
{
    return RGB{avg_hsv(std::span<const uint16_t>{pixels.data(), pixels.size()})};
}
