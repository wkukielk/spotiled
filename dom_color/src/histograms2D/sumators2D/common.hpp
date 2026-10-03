#ifndef SUMATORS2D_COMMON_HPP
#define SUMATORS2D_COMMON_HPP

#include "histogram2D.hpp"
#include "histogram2D_aux.hpp"

namespace hist2D {
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
