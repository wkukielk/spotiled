#include "non_linear.hpp"
#include "depth_it.hpp"
#include "lap_it.hpp"
#include "wraparound_it.hpp"
#include <bit>
#include <functional>
#include <numeric>
#include <ranges>

namespace hist {
namespace sumators {
namespace {
auto neighbourhood(const std::span<const MyCluster> &clusters,
                   const MyCluster *center, uint8_t depth) {
    std::ptrdiff_t radius = (1 << depth) - 1;
    auto left = lap_iterator{
        wraparound_iterator{clusters,
                            clusters.begin() + (center - clusters.data())} -
        (int)radius};
    return std::views::counted(depth_iterator{left, -radius}, 2 * radius + 1) |
           std::views::common;
}
} // namespace

NonLinear::NonLinear(std::span<const MyCluster> clusters,
                     uint16_t exclusive_upper_bound)
    : clusters{clusters}, upper_bound{exclusive_upper_bound} {}

uint64_t NonLinear::count(CenterWindow w) const {
    if (w.depth > sizeof(weights) - 1)
        return 0;
    auto clusters_around = neighbourhood(clusters, w.center, w.depth);
    return std::transform_reduce(
        clusters_around.begin(), clusters_around.end(), uint64_t{0},
        std::plus{}, [](auto cluster) -> uint64_t {
            auto weight = weights[std::bit_width((size_t)cluster.distance)];
            return weight * cluster.value.value.count;
        });
}

uint64_t NonLinear::sum(CenterWindow w) const {
    if (w.depth > sizeof(weights) - 1)
        return 0;
    uint64_t turn = upper_bound >> hshift;
    auto clusters_around = neighbourhood(clusters, w.center, w.depth);
    return std::transform_reduce(
        clusters_around.begin(), clusters_around.end(), uint64_t{0},
        std::plus{}, [turn](auto cluster) -> uint64_t {
            auto weight = weights[std::bit_width((size_t)cluster.distance)];
            auto [value, lap] = cluster.value;
            return weight * (value.sum + lap * value.count * turn);
        });
}
}; // namespace sumators
}; // namespace hist
