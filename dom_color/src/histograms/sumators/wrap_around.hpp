#ifndef SUMATORS_WRAP_AROUND
#define SUMATORS_WRAP_AROUND

#include "./common.hpp"
#include "histogram_aux.hpp"
#include <span>

namespace hist {
namespace sumators {
class WrapAround {
  public:
    using window_t = BasicWindow;

    WrapAround(std::span<const MyCluster>, uint16_t exclusive_upper_bound);
    uint32_t count(BasicWindow w) const noexcept;
    uint32_t sum(BasicWindow w) const noexcept;

  protected:
    const std::span<const MyCluster> clusters;
    const uint16_t upper_bound;
};

static_assert(Sumator<WrapAround>);
}; // namespace sumators
}; // namespace hist

#endif
