#include "basic.hpp"
#include <functional>
#include <numeric>

namespace hist {
    namespace sumators {
        Basic::Basic(std::span<const MyCluster> clusters) : clusters{clusters} {}

        uint32_t Basic::interval_count(const MyCluster* first, uint8_t size) const {
            if (first < clusters.data() || first + size > clusters.data() + clusters.size())
                return 0;
            return std::transform_reduce(first, first + size, uint32_t{0},
                std::plus{}, [](const MyCluster& cluster) { return cluster.count; });
        }

        uint32_t Basic::interval_sum(const MyCluster* first, uint8_t size) const {
            if (first < clusters.data() || first + size > clusters.data() + clusters.size())
                return 0;
            return std::transform_reduce(first, first + size, uint32_t{0},
                std::plus{}, [](const MyCluster& cluster) { return cluster.sum; });
        }
    };
};
