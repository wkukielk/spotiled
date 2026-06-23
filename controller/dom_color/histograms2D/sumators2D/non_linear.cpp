#include "non_linear.hpp"
#include <algorithm>

namespace hist2D {
namespace sumators {
const uint8_t NonLinear::weights[5][5] = {{2, 3, 3, 3, 2},
                                          {3, 4, 6, 4, 3},
                                          {3, 6, 12, 6, 3},
                                          {3, 4, 6, 4, 3},
                                          {2, 3, 3, 3, 2}};

NonLinear::NonLinear(const Histogram &histogram)
    : Basic{histogram.get_clusters()}, histogram{histogram} {}

uint64_t NonLinear::neighbourhood_count(const MyCluster *center,
                                        uint8_t depth) const {
    auto &clusters = *histogram.get_clusters();
    auto [x_row, y_row] = histogram.get_index(center);
    auto offsets = get_cluster_offsets(center, depth);
    uint64_t count = 0;
    for (int8_t delta_y = offsets.y.lower; delta_y != offsets.y.upper;
         ++delta_y)
        for (int8_t delta_x = offsets.x.lower; delta_x != offsets.x.upper;
             ++delta_x) {
            count += weights[2 + delta_y][2 + delta_x] *
                     clusters[y_row + delta_y][x_row + delta_x].count;
        }
    return count;
}

Sum64 NonLinear::neighbourhood_sum(const MyCluster *center,
                                   uint8_t depth) const {
    auto &clusters = *histogram.get_clusters();
    auto [x_row, y_row] = histogram.get_index(center);
    auto offsets = get_cluster_offsets(center, depth);
    Sum64 sum{};
    for (int8_t delta_y = offsets.y.lower; delta_y != offsets.y.upper;
         ++delta_y)
        for (int8_t delta_x = offsets.x.lower; delta_x != offsets.x.upper;
             ++delta_x) {
            sum.x += weights[2 + delta_y][2 + delta_x] *
                     (uint64_t)clusters[y_row + delta_y][x_row + delta_x].sum_x;
            sum.y += weights[2 + delta_y][2 + delta_x] *
                     (uint64_t)clusters[y_row + delta_y][x_row + delta_x].sum_y;
        }
    return sum;
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
