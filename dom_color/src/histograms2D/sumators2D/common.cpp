#include "common.hpp"
#include <algorithm>

namespace hist2D {
namespace {
// Values covered by clusters [lower, upper) along one axis.
Bounds axis_bounds(int lower, int upper, uint16_t exclusive_upper_bound) {
    return {.lower = (uint16_t)(lower * exclusive_upper_bound / sz),
            .upper = (uint16_t)(upper * exclusive_upper_bound / sz)};
}
} // namespace

ValueBounds get_bounds(const Histogram &histogram, BasicWindow w) {
    auto [x, y] = histogram.get_index(w.topleft);
    return {.x = axis_bounds(x, x + w.size, histogram.get_upper_bound_x()),
            .y = axis_bounds(y, y + w.size, histogram.get_upper_bound_y())};
}

ValueBounds get_bounds(const Histogram &histogram, CenterWindow w) {
    auto [x, y] = histogram.get_index(w.center);
    auto lower = [&](int index) { return std::max(index - w.depth, 0); };
    auto upper = [&](int index) {
        return std::min(index + w.depth + 1, (int)sz);
    };
    return {.x = axis_bounds(lower(x), upper(x), histogram.get_upper_bound_x()),
            .y = axis_bounds(lower(y), upper(y), histogram.get_upper_bound_y())};
}
} // namespace hist2D
