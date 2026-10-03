#include "basic.hpp"

namespace hist2D {
    namespace analyzers {
        Basic::Basic(const Histogram& histogram, const sumators::Basic& sumator) :
            histogram{histogram}, sumator{sumator},
            clusters{histogram.get_clusters()} {}

        Dominant Basic::get_dominant() const {
            auto dominant = Dominant{};
            auto [largest, count, size] = get_max_cluster();
            if (largest) {
                dominant.value = get_representant_value(largest, size);
                dominant.bounds = get_cluster_bounds(largest, size);
                dominant.count = count;
            }
            return dominant;
        }

        _MaxCluster Basic::get_max_cluster() const {
            auto curr_precission = halvings;
            uint8_t cluster_size = 1;
            uint32_t max_count = 0;
            const MyCluster* begin = (MyCluster*)clusters;
            const MyCluster* largest = begin;
            const MyCluster* last = (MyCluster*)(clusters + 1);
            do {
                const MyCluster* current = begin;
                auto square_end_offset = cluster_size * sz;
                do {
                    auto row_end = current + sz;
                    do {
                        auto current_count = sumator.interval_count(current, cluster_size);
                        if (current_count > max_count) {
                            largest = current;
                            max_count = current_count;
                        }
                    } while ( current++ + cluster_size != row_end );
                    current = row_end;
                } while ( current + square_end_offset != last + sz );
                cluster_size *= 2;
            } while (max_count < histogram.get_count() / (basesz * basesz) && curr_precission-- != 0);
            cluster_size /= 2;
            return {
                .topleft = largest,
                .count = max_count,
                .square_size = cluster_size
            };
        }

        Value Basic::get_representant_value(const MyCluster* topleft, uint8_t square_size) const {
            auto count = sumator.interval_count(topleft, square_size);
            auto sum = sumator.interval_sum(topleft, square_size);
            uint16_t x = count != 0 ? (sum.x / count) << hshift : 0;
            uint16_t y = count != 0 ? (sum.y / count) << hshift : 0;
            return Value{ .x = x, .y = y };
        }

        ValueBounds Basic::get_cluster_bounds(const MyCluster* topleft, uint8_t square_size) const {
            auto [x_row, y_row] = histogram.get_index(topleft);
            uint16_t x_lower = x_row * histogram.get_upper_bound_x() / sz;
            uint16_t x_upper = (x_row + square_size) * histogram.get_upper_bound_x() / sz;
            uint16_t y_lower = y_row * histogram.get_upper_bound_y() / sz;
            uint16_t y_upper = (y_row + square_size) * histogram.get_upper_bound_y() / sz;
            return {
                .x = Bounds {
                    .lower = x_lower,
                    .upper = x_upper
                },
                .y = Bounds {
                    .lower = y_lower,
                    .upper = y_upper
                }
            };
        }
    };
};
