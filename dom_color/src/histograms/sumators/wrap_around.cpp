#include "wrap_around.hpp"
#include "lap_it.hpp"
#include "wraparound_it.hpp"
#include <cstddef>
#include <functional>
#include <numeric>
#include <ranges>

namespace hist {
namespace sumators {
namespace {
// The iterator keeps a pointer to `clusters`, so it must outlive the window.
auto window(const std::span<const MyCluster> &clusters, const MyCluster *left,
            size_t length) {
    auto start = lap_iterator{wraparound_iterator{
        clusters, clusters.begin() + (left - clusters.data())}};
    return std::views::counted(start, (std::ptrdiff_t)length) |
           std::views::common;
}
} // namespace

WrapAround::WrapAround(std::span<const MyCluster> clusters,
                       uint16_t exclusive_upper_bound)
    : clusters{clusters}, upper_bound{exclusive_upper_bound} {}

uint32_t WrapAround::wrap_interval_count(const MyCluster *left,
                                         size_t length) const {
    auto interval = window(clusters, left, length);
    return std::transform_reduce(
        interval.begin(), interval.end(), uint32_t{0}, std::plus{},
        [](auto cluster) { return cluster.value.count; });
}

uint64_t WrapAround::wrap_interval_sum(const MyCluster *left,
                                       size_t length) const {
    uint64_t turn = upper_bound >> hshift;
    auto interval = window(clusters, left, length);
    return std::transform_reduce(
        interval.begin(), interval.end(), uint64_t{0}, std::plus{},
        [turn](auto cluster) -> uint64_t {
            return cluster.value.sum + cluster.lap * cluster.value.count * turn;
        });
}
}; // namespace sumators
}; // namespace hist
