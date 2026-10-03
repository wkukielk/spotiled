#ifndef SUMATORS_BASIC_HPP
#define SUMATORS_BASIC_HPP

#include "common.hpp"
#include "histogram.hpp"
#include "histogram_aux.hpp"
#include <span>

namespace hist {
namespace sumators {
class Basic {
  public:
    using window_t = BasicWindow;

    Basic(std::span<const MyCluster> clusters);
    uint32_t count(BasicWindow window) const noexcept;
    uint32_t sum(BasicWindow window) const noexcept;

  protected:
    using MappingFn = decltype(MyCluster::sum) (*)(const MyCluster &);
    uint32_t transform_sum(BasicWindow window, MappingFn fn) const noexcept;

    const std::span<const MyCluster> clusters;
};

static_assert(Sumator<Basic>);
}; // namespace sumators
}; // namespace hist

#endif
