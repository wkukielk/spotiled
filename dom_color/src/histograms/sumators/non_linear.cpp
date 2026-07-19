#include "non_linear.hpp"
#include "wraparound_it.hpp"


namespace hist {
    namespace sumators {
        const uint8_t NonLinear::weights[] = {24, 8, 4, 2, 1};
        const uint8_t NonLinear::offsets[] = {0, 2, 5, 11, 23};

        uint64_t NonLinear::neighbourhood_count(const MyCluster* center, uint8_t depth)
                const {
            if (depth > sizeof(weights) - 1)
                return 0;
            auto curr = wraparound_iterator{clusters, center};
            curr -= ((1<<depth) - 1);
            auto curr_depth = depth + 1;
            auto curr_width = 1 << (depth - 1);
            uint64_t count = 0;
            while (--curr_depth != 0) {
                count += weights[curr_depth] *
                    wrap_interval_count(curr.base(), curr_width);
                count += weights[curr_depth] *
                    wrap_interval_count((curr+offsets[curr_depth]).base(), curr_width);
                curr += curr_width;
                curr_width /= 2;
            }
            count += weights[0] * (center -> count);
            return count;
        }

        uint64_t NonLinear::neighbourhood_sum(const MyCluster* center, uint8_t depth)
                const {
            if (depth > sizeof(weights) - 1)
                return 0;
            auto curr = wraparound_iterator{clusters, center};
            curr -= ((1<<depth) - 1);
            auto curr_depth = depth + 1;
            auto curr_width = 1 << (depth - 1);
            uint64_t sum = 0;
            while (--curr_depth != 0) {
                sum += weights[curr_depth] *
                    (uint64_t)wrap_interval_sum(curr.base(), curr_width);
                sum += weights[curr_depth] *
                    (uint64_t)wrap_interval_sum((curr + offsets[curr_depth]).base(),
                                                curr_width);
                curr += curr_width;
                curr_width /= 2;
            }
            sum += weights[0] * (uint64_t)(center -> sum);
            return sum;
        }
    };
};
