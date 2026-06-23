#include "basic.hpp"


namespace hist {
    namespace sumators {
        Basic::Basic(const ClusterArray& clusters) :
            begin{clusters.begin()}, end{clusters.end()} {}

        uint32_t Basic::interval_count(const MyCluster* clusters, uint8_t size) const {
            size_t count = 0;
            if (clusters >= begin && clusters + size <= end)
                for (auto cluster = clusters; cluster != clusters + size; ++cluster)
                    count += cluster -> count;
            return count;
        }

        uint32_t Basic::interval_sum(const MyCluster* clusters, uint8_t size) const {
            uint32_t sum = 0;
            if (clusters >= begin && clusters + size <= end)
                for (auto cluster = clusters; cluster != clusters + size; ++cluster)
                    sum += cluster -> sum;
            return sum;
        }
    };
};
