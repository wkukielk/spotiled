#include "non_linear.hpp"
#include "rect_it.hpp"
#include <algorithm>
#include <functional>
#include <numeric>

namespace hist2D {
namespace sumators {
const uint8_t NonLinear::weights[5][5] = {{2, 3, 3, 3, 2},
                                          {3, 4, 6, 4, 3},
                                          {3, 6, 12, 6, 3},
                                          {3, 4, 6, 4, 3},
                                          {2, 3, 3, 3, 2}};

NonLinear::NonLinear(const Histogram &histogram) : histogram{histogram} {}

namespace {
// Sum of weights * value_of(cluster) over `area`, weighted by each cluster's
// offset from `center`.
template <typename ValueOf>
uint64_t weighted_sum(const MyCluster (&clusters)[sz][sz], Index center,
                      IndexRect area, ValueOf value_of) {
    auto [x, y] = area;
    auto neighbourhood = rect(&clusters[y.lower][x.lower], x.upper - x.lower,
                              y.upper - y.lower, sz);
    return std::transform_reduce(
        neighbourhood.begin(), neighbourhood.end(), uint64_t{0}, std::plus{},
        [&](auto cluster) -> uint64_t {
            auto dy = y.lower + cluster.row - center.y;
            auto dx = x.lower + cluster.col - center.x;
            return NonLinear::weights[2 + dy][2 + dx] *
                   (uint64_t)value_of(cluster.value);
        });
}
} // namespace

uint64_t NonLinear::count(CenterWindow w) const {
    auto center = histogram.get_index(w.center);
    return weighted_sum(*histogram.get_clusters(), center,
                        clipped_area(center, w.depth),
                        [](const MyCluster &cluster) { return cluster.count; });
}

Sum64 NonLinear::sum(CenterWindow w) const {
    auto &clusters = *histogram.get_clusters();
    auto center = histogram.get_index(w.center);
    auto area = clipped_area(center, w.depth);
    return {.x = weighted_sum(clusters, center, area,
                              [](const MyCluster &cluster) { return cluster.sum_x; }),
            .y = weighted_sum(clusters, center, area,
                              [](const MyCluster &cluster) { return cluster.sum_y; })};
}
}; // namespace sumators
}; // namespace hist2D
