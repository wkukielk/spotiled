#include "basic.hpp"
#include <functional>
#include <numeric>

namespace hist {
namespace sumators {
Basic::Basic(std::span<const MyCluster> clusters) : clusters{clusters} {}

uint32_t Basic::count(BasicWindow w) const noexcept {
    auto f = [](const MyCluster &cluster) { return (uint32_t)cluster.count; };
    return transform_sum(w, f);
}

uint32_t Basic::sum(BasicWindow w) const noexcept {
    auto get_sum = [](const MyCluster &cluster) { return cluster.sum; };
    return transform_sum(w, get_sum);
}

uint32_t Basic::transform_sum(BasicWindow w, MappingFn fn) const noexcept {
    if (w.first < clusters.data() ||
        w.first + w.size > clusters.data() + clusters.size())
        return 0;
    return std::transform_reduce(w.first, w.first + w.size, uint32_t{0},
                                 std::plus{}, fn);
}
}; // namespace sumators
}; // namespace hist
