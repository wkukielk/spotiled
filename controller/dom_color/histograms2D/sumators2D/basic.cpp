#include "basic.hpp"


namespace hist2D {
    namespace sumators {
        Basic::Basic(const MyCluster (*clusters)[sz][sz]) :
            begin{(MyCluster*)clusters}, end{(MyCluster*)(clusters + 1)} {}

        uint32_t Basic::interval_count(const MyCluster* topleft, uint8_t square_size) const {
            uint32_t count = 0;
            auto y_end = topleft + square_size * sz;
            auto& row_begin = topleft;
            while (row_begin != y_end) {
                for (auto cluster = row_begin; cluster != row_begin + square_size; ++cluster)
                    count += cluster -> count;
                row_begin += sz;
            }
            return count;
        }

        Sum Basic::interval_sum(const MyCluster* topleft, uint8_t square_size) const {
            uint32_t sum_x = 0;
            uint32_t sum_y = 0;
            auto y_end = topleft + square_size * sz;
            auto& row_begin = topleft;
            while (row_begin != y_end) {
                for (auto cluster = row_begin; cluster != row_begin + square_size; ++cluster) {
                    sum_x += cluster -> sum_x;
                    sum_y += cluster -> sum_y;
                }
                row_begin += sz;
            }
            return { .x = sum_x, .y = sum_y };
        }
    };
};
