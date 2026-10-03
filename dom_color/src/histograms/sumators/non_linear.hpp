#ifndef SUMATORS_NON_LINEAR_HPP
#define SUMATORS_NON_LINEAR_HPP

#include "./common.hpp"
#include "histogram_aux.hpp"
#include <span>

namespace hist {
namespace sumators {
class NonLinear {
  public:
    using window_t = CenterWindow;

    NonLinear(std::span<const MyCluster>, uint16_t exclusive_upper_bound);
    uint64_t count(CenterWindow w) const;
    uint64_t sum(CenterWindow w) const;
    static uint8_t get_magic_number() { return magic_number; };
    static const uint8_t weights[];

  protected:
    static const uint8_t magic_number = 88;
    const std::span<const MyCluster> clusters;
    const uint16_t upper_bound;
};

static_assert(Sumator<NonLinear>);

}; // namespace sumators
}; // namespace hist

#endif
