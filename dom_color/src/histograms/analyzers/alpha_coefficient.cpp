#include "alpha_coefficient.hpp"


namespace hist {
    namespace analyzers {
        AlphaCoefficient6::AlphaCoefficient6(uint8_t alpha) : alpha{alpha} {
            alpha6 = (uint32_t(5 * alpha + 249) << alpha_shift) / 254;
        }

        void AlphaCoefficient6::set_alpha(uint8_t alpha) {
            this -> alpha = alpha;
            alpha6 = (uint32_t(5 * alpha + 249) << alpha_shift) / 254;
        }

        bool AlphaCoefficient6::is_count_enough(uint32_t count,
                uint32_t total) const {
            return (uint64_t)count * 6 << alpha_shift >= alpha6 * (uint64_t)total;
        }

        AlphaCoefficient48::AlphaCoefficient48(uint8_t alpha) : alpha{alpha} {
            alpha48 = (uint32_t(47 * alpha + 207) << alpha_shift) / 254;
        }

        void AlphaCoefficient48::set_alpha(uint8_t alpha) {
            this -> alpha = alpha;
            alpha48 = (uint32_t(47 * alpha + 207) << alpha_shift) / 254;
        }

        bool AlphaCoefficient48::is_count_enough(uint32_t count, uint32_t boundary_count) const {
            return (uint64_t)count << alpha_shift >= alpha48 * (uint64_t)boundary_count;
        }
    };
};
