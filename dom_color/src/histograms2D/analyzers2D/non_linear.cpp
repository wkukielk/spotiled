#include "non_linear.hpp"

namespace hist2D {
    namespace analyzers {
        NonLinear::NonLinear(const Histogram& histogram,
                const sumators::NonLinear& sumator) :
            histogram{histogram}, sumator{sumator},
            clusters{histogram.get_clusters()} {}

        Dominant NonLinear::get_dominant() const {
            auto dominant = Dominant{};
            auto [largest, count, depth] = get_max_cluster();
            if (largest) {
                dominant.bounds = get_bounds(histogram, CenterWindow{largest, depth});
                dominant.count = count;
                dominant.value = get_representant_value(largest, depth);
            }
            return dominant;
        }

        _nonlinear::MaxCluster NonLinear::get_max_cluster() const {
            uint8_t current_depth = 0;
            uint32_t max_count = 0;
            const MyCluster* begin = (MyCluster*)clusters;
            const MyCluster* largest = begin;
            const MyCluster* last = (MyCluster*)(clusters + 1);
            do {
                const MyCluster* current = begin;
                do {
                    auto row_end = current + sz;
                    do {
                        auto current_count = sumator.count({current, current_depth});
                        if (current_count > max_count) {
                            largest = current;
                            max_count = current_count;
                        }
                    } while ( ++current != row_end );
                } while (current != last);
            } while (max_count < histogram.get_count() / (basesz * basesz) && ++current_depth != halvings);
            if (current_depth != halvings)
                return {
                    .cluster = largest,
                    .count = max_count,
                    .depth = current_depth
                };
            else
                return {};
        }

        uint32_t NonLinear::get_boundary_count() const {
            return sumator.get_magic_number() * histogram.get_count() / (sz * sz);
        }

        Value NonLinear::get_representant_value(const MyCluster* center, uint8_t depth) const {
            auto sum = sumator.sum({center, depth});
            auto count = sumator.count({center, depth});
            uint16_t value_x = count != 0 ? (sum.x << hshift) / count : 0;
            uint16_t value_y = count != 0 ? (sum.y << hshift) / count : 0;
            return Value {
                .x = value_x,
                .y = value_y
            };
        }
    };
};
