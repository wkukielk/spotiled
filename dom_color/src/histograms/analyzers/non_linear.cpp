#include "non_linear.hpp"
#include "wraparound_it.hpp"


namespace hist {
    namespace analyzers {
        NonLinear::NonLinear(const Histogram& histogram,
            const sumators::NonLinear& sumator, uint8_t alpha) :
            histogram{histogram}, sumator{sumator},
            clusters{histogram.get_clusters()}, alpha48{alpha} {}

        Dominant NonLinear::get_dominant() const {
            auto dominant = Dominant{};
            auto max_cluster = get_max_cluster();
            if (max_cluster.cluster) {
                dominant.bounds.lower = get_lower_bound(max_cluster);
                dominant.bounds.upper = get_upper_bound(max_cluster);
                dominant.count = max_cluster.count;
                dominant.value = get_representant_value(max_cluster.cluster, max_cluster.depth);
            }
            return dominant;
        }

        _nonlinear::MaxCluster NonLinear::get_max_cluster() const {
            if (histogram.get_count() == 0)
                return {};
            uint8_t current_depth = 0;
            const MyCluster* const begin = clusters.data();
            const MyCluster* const end = clusters.end();
            const MyCluster* largest = begin;
            uint32_t max_count = 0;
            do {
                max_count = 0;
                const MyCluster* current = begin;
                do {
                    auto current_count = sumator.neighbourhood_count(current, current_depth);
                    if (current_count > max_count) {
                        largest = current;
                        max_count = current_count;
                    }
                } while (++current != end);
            } while (not alpha48.is_count_enough(max_count, get_boundary_count()) &&
                ++current_depth != halvings);
            if (current_depth != halvings) {
                return {
                    .cluster = largest,
                    .count = max_count,
                    .depth = current_depth
                };
            } else return {};
        }

        uint16_t NonLinear::get_lower_bound(const _nonlinear::MaxCluster& cluster)
                const {
            auto [center, count, depth] = cluster;
            const MyCluster* const begin = clusters.begin();
            auto index = center - (1<<depth) + 1 - begin;
            if (index < 0)
                index += sz;
            return histogram.get_upper_bound() * index / sz;
        }

        uint16_t NonLinear::get_upper_bound(const _nonlinear::MaxCluster& cluster)
                const {
            auto [center, count, depth] = cluster;
            const MyCluster* const begin = clusters.begin();
            auto index = center + (1<<depth) - begin;
            if (index >= histogram.get_upper_bound()) {
                index -= sz;
            }
            return histogram.get_upper_bound() * index / sz;
        }

        uint32_t NonLinear::get_boundary_count() const {
            return sumator.get_magic_number() * histogram.get_count() / sz;
        }

        uint16_t NonLinear::get_representant_value(const MyCluster* clusters, uint8_t depth) const {
            uint64_t sum = sumator.neighbourhood_sum(clusters, depth);
            uint64_t count = sumator.neighbourhood_count(clusters, depth);
            uint64_t boundary = count * histogram.get_upper_bound();
            if (sum >= boundary) {
                sum -= boundary;
            }
            return count != 0 ? (sum << hshift) / count  : 0;
        }
    };
};
