#include "alpha_coefficient.hpp"


namespace hist {
    namespace analyzers {
        // count >= min + (max - min) * (alpha - 1) / 254, without division
        bool AlphaCoefficient::is_count_enough(uint64_t count, uint64_t min_count,
                uint64_t max_count) const {
            int64_t threshold = (int64_t)min_count * 254 +
                ((int64_t)max_count - min_count) * (alpha - 1);
            return (int64_t)count * 254 >= threshold;
        }
    };
};
