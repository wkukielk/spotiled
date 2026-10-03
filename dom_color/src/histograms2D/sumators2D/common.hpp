#ifndef SUMATORS2D_COMMON_HPP
#define SUMATORS2D_COMMON_HPP

#include "histogram2D.hpp"
#include "histogram2D_aux.hpp"
#include <algorithm>

namespace hist2D {
// Cluster indices [lower, upper) along one axis.
struct IndexRange {
    int lower;
    int upper;
};

// A rectangle of clusters: columns x, rows y.
struct IndexRect {
    IndexRange x;
    IndexRange y;
};

inline IndexRange intersect(IndexRange a, IndexRange b) {
    return {std::max(a.lower, b.lower), std::min(a.upper, b.upper)};
}

inline IndexRect intersect(IndexRect a, IndexRect b) {
    return {intersect(a.x, b.x), intersect(a.y, b.y)};
}

// The clusters within `depth` of `center` that lie inside the histogram.
inline IndexRect clipped_area(Index center, uint8_t depth) {
    auto window = IndexRect{.x = {center.x - depth, center.x + depth + 1},
                            .y = {center.y - depth, center.y + depth + 1}};
    auto all = IndexRect{.x = {0, (int)sz}, .y = {0, (int)sz}};
    return intersect(window, all);
}

// A size x size square of clusters.
struct BasicWindow {
    const MyCluster *topleft;
    uint8_t size;
};

// The clusters within `depth` of `center` in both directions.
struct CenterWindow {
    const MyCluster *center;
    uint8_t depth;
};

ValueBounds get_bounds(const Histogram &histogram, BasicWindow w);
// The window is clipped at the histogram's edges.
ValueBounds get_bounds(const Histogram &histogram, CenterWindow w);
} // namespace hist2D

#endif
