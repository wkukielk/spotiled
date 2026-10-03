#include "common.hpp"

namespace hist2D {
namespace {
// Values covered by the clusters of `range` along one axis.
Bounds axis_bounds(IndexRange range, uint16_t exclusive_upper_bound) {
    return {.lower = (uint16_t)(range.lower * exclusive_upper_bound / sz),
            .upper = (uint16_t)(range.upper * exclusive_upper_bound / sz)};
}
} // namespace

ValueBounds get_bounds(const Histogram &histogram, BasicWindow w) {
    auto [x, y] = histogram.get_index(w.topleft);
    return {.x = axis_bounds({x, x + w.size}, histogram.get_upper_bound_x()),
            .y = axis_bounds({y, y + w.size}, histogram.get_upper_bound_y())};
}

ValueBounds get_bounds(const Histogram &histogram, CenterWindow w) {
    auto clipped = clipped_area(histogram.get_index(w.center), w.depth);
    return {.x = axis_bounds(clipped.x, histogram.get_upper_bound_x()),
            .y = axis_bounds(clipped.y, histogram.get_upper_bound_y())};
}
} // namespace hist2D
