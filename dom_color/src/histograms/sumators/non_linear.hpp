#ifndef SUMATORS_NON_LINEAR_HPP
#define SUMATORS_NON_LINEAR_HPP

#include "./common.hpp"
#include "histogram_aux.hpp"
#include <functional>
#include <numeric>
#include <ranges>
#include <span>

namespace hist {
namespace sumators {
class NonLinear {
  public:
    using window_t = CenterWindow;

    NonLinear(std::span<const MyCluster>, uint16_t exclusive_upper_bound);
    uint64_t count(CenterWindow w) const;
    uint64_t sum(CenterWindow w) const;
    static constexpr uint8_t weights[] = {24, 8, 4, 2, 1};

    // Total weight of a neighbourhood of `depth`: the centre, plus 2^i
    // clusters at weights[i] for each ring i.
    static constexpr uint32_t magic_number(uint8_t depth) {
        auto rings = std::views::iota(1, depth + 1);
        return std::transform_reduce(
            rings.begin(), rings.end(), uint32_t{weights[0]}, std::plus{},
            [](int i) { return uint32_t{weights[i]} << i; });
    }

  protected:
    const std::span<const MyCluster> clusters;
    const uint16_t upper_bound;
};

static_assert(Sumator<NonLinear>);
static_assert(NonLinear::magic_number(4) == 88);

}; // namespace sumators
}; // namespace hist

#endif
