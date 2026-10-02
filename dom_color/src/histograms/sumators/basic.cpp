#include "basic.hpp"
#include <functional>
#include <numeric>

namespace hist {
    namespace sumators {
        Basic::Basic(std::span<const MyCluster> clusters) :
            begin{clusters.data()}, end{clusters.data() + clusters.size()} {}

        uint32_t Basic::interval_count(const MyCluster* clusters, uint8_t size) const {
            if (clusters < begin || clusters + size > end)
                return 0;
            return std::transform_reduce(clusters, clusters + size, uint32_t{0},
                std::plus{}, [](const MyCluster& cluster) { return cluster.count; });
        }

        uint32_t Basic::interval_sum(const MyCluster* clusters, uint8_t size) const {
            if (clusters < begin || clusters + size > end)
                return 0;
            return std::transform_reduce(clusters, clusters + size, uint32_t{0},
                std::plus{}, [](const MyCluster& cluster) { return cluster.sum; });
        }
    };
};
