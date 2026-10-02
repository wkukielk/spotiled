#include "wrap_around.hpp"


namespace hist {
    namespace sumators {
        WrapAround::WrapAround(std::span<const MyCluster> clusters,
                uint16_t exclusive_upper_bound) :
            Basic{clusters}, clusters{clusters}, upper_bound{exclusive_upper_bound} {}
        
        uint32_t WrapAround::wrap_interval_count(const MyCluster* left, size_t length)
                const {
            size_t count = 0;
            if (left + length <= end) {
                for (auto current = left; current != left + length; ++current)
                    count += current -> count;
            } else {
                for (auto current = left; current != end; ++current)
                    count += current -> count;
                for (auto current = begin; current != begin + length - (end - left);
                        ++current)
                    count += current -> count;
            }
            return count;
        }

        uint64_t WrapAround::wrap_interval_sum(const MyCluster* left, size_t length)
                const {
            uint64_t sum = 0;
            if (left + length <= end)
                for (auto curr = left; curr != left + length; ++curr)
                    sum += curr -> sum;
            else {
                for (auto curr = left; curr != end; ++curr)
                    sum += curr -> sum;
                for (auto curr = begin; curr != begin + length - (end - left); ++curr)
                    sum += curr -> sum + curr -> count * (upper_bound >> hshift);
            }
            return sum;
        }
    };
};
