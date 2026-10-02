#include "wrap_around.hpp"


namespace hist {
    namespace analyzers {
        WrapAround::WrapAround(const Histogram& histogram,
                const sumators::WrapAround& sumator, uint8_t alpha) :
            histogram{histogram}, sumator{sumator},
            clusters{histogram.get_clusters()}, alpha6{alpha} {}
        
        Dominant WrapAround::get_dominant() const {
            auto dominant =  Dominant{};
            const MyCluster* const begin = clusters.begin();
            auto [largest, count, size] = get_max_cluster();
            if (largest) {
                dominant.bounds.lower = histogram.get_upper_bound() * (largest - begin) / sz;
                dominant.bounds.upper =
                    histogram.get_upper_bound() * (largest - begin + size) / sz;
                dominant.count = count;
                dominant.value = get_representant_value(largest, size);
            }
            return dominant;
        }

        _wraparound::MaxCluster WrapAround::get_max_cluster() const {
            if (histogram.get_count() == 0)
                return {};
            auto precission = Precission{halvings};
            const MyCluster* const begin = clusters.data();
            const MyCluster* const end = clusters.end();
            const MyCluster* largest = begin;
            uint32_t max_count = 0;
            do {
                max_count = 0;
                const MyCluster* current = end - precission.get_cluster_size() + 1;
                for (; current != end; ++current) {
                    auto current_count = sumator.wrap_interval_count(current, precission.get_cluster_size());
                    if (current_count > max_count) {
                        largest = current;
                        max_count = current_count;
                    }
                }
                current = begin;
                do {
                    auto current_count = sumator.wrap_interval_count(current, precission.get_cluster_size());
                    if (current_count > max_count) {
                        largest = current;
                        max_count = current_count;
                    }
                } while ( current++ + precission.get_cluster_size() != end );
            } while (not alpha6.is_count_enough(max_count, histogram.get_count()) &&
                (--precission).get_precission() != 0);
            if (precission.get_precission())
                return {
                    .cluster = largest,
                    .count = max_count,
                    .size = (uint8_t)precission.get_cluster_size()
                };
            else
                return {};
        }

        uint16_t WrapAround::get_representant_value(const MyCluster* clusters, uint8_t length) const {
            uint64_t sum = sumator.wrap_interval_sum(clusters, length);
            uint32_t count = sumator.wrap_interval_count(clusters, length);
            uint64_t boundary = (uint64_t)count * (histogram.get_upper_bound() >> hshift);
            if (sum >= boundary) {
                sum -= boundary;
            }
            return count != 0 ? (sum / count) << hshift : 0;
        }
    };
};
