#include "common.hpp"
#include "histogram.hpp"

namespace hist {
Bounds get_bounds(const Histogram &histogram, BasicWindow w) {
    auto begin = histogram.get_clusters().data();
    auto upper = histogram.get_upper_bound();
    auto bounds = Bounds{};
    bounds.lower = upper * (w.first - begin) / sz;
    bounds.upper = upper * (w.first - begin + w.size) / sz;
    return bounds;
};

Bounds get_bounds(const Histogram &histogram, CenterWindow w) {
    auto begin = histogram.get_clusters().data();

    auto index_low = w.center - (1 << w.depth) + 1 - begin;
    if (index_low < 0)
        index_low += sz;

    auto index_up = w.center + (1 << w.depth) - begin;
    if (index_up >= (std::ptrdiff_t)sz)
        index_up -= sz;

    auto bounds = Bounds{};
    bounds.lower = histogram.get_upper_bound() * index_low / sz;
    bounds.upper = histogram.get_upper_bound() * index_up / sz;
    return bounds;
};
} // namespace hist
