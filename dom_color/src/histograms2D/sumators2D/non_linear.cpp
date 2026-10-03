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

NonLinear::NonLinear(const Histogram &histogram)
    : Basic{histogram.get_clusters()}, histogram{histogram} {}

namespace {
// Sum of weights * value_of(cluster) over the clusters at `offsets` around
// `center`.
template <typename ValueOf>
uint64_t weighted_sum(const MyCluster (&clusters)[sz][sz], Index center,
                      ClusterOffsets offsets, ValueOf value_of) {
    auto [x, y] = offsets;
    auto neighbourhood = rect(&clusters[center.y + y.lower][center.x + x.lower],
                              x.upper - x.lower, y.upper - y.lower, sz);
    return std::transform_reduce(
        neighbourhood.begin(), neighbourhood.end(), uint64_t{0}, std::plus{},
        [&](auto cluster) -> uint64_t {
            auto dy = y.lower + cluster.row;
            auto dx = x.lower + cluster.col;
            return NonLinear::weights[2 + dy][2 + dx] *
                   (uint64_t)value_of(cluster.value);
        });
}
} // namespace

uint64_t NonLinear::neighbourhood_count(const MyCluster *center,
                                        uint8_t depth) const {
    return weighted_sum(*histogram.get_clusters(), histogram.get_index(center),
                        get_cluster_offsets(center, depth),
                        [](const MyCluster &cluster) { return cluster.count; });
}

Sum64 NonLinear::neighbourhood_sum(const MyCluster *center,
                                   uint8_t depth) const {
    auto &clusters = *histogram.get_clusters();
    auto index = histogram.get_index(center);
    auto offsets = get_cluster_offsets(center, depth);
    return {.x = weighted_sum(clusters, index, offsets,
                              [](const MyCluster &cluster) { return cluster.sum_x; }),
            .y = weighted_sum(clusters, index, offsets,
                              [](const MyCluster &cluster) { return cluster.sum_y; })};
}

ClusterOffsets NonLinear::get_cluster_offsets(const MyCluster *center,
                                              uint8_t depth) const {
    auto [x_row, y_row] = histogram.get_index(center);
    auto lower_x_offset = std::max<int8_t>(-depth, 0 - x_row);
    auto upper_x_offset = std::min<int8_t>(depth + 1, sz - x_row);
    auto lower_y_offset = std::max<int8_t>(-depth, 0 - y_row);
    auto upper_y_offset = std::min<int8_t>(depth + 1, sz - y_row);
    return ClusterOffsets{
        .x = Offsets{.lower = lower_x_offset, .upper = upper_x_offset},
        .y = Offsets{.lower = lower_y_offset, .upper = upper_y_offset}};
}
}; // namespace sumators
}; // namespace hist2D
