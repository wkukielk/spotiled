#ifndef SUMATORS_BASIC_HPP
#define SUMATORS_BASIC_HPP

#include "histogram_aux.hpp"
#include <span>

namespace hist {
namespace sumators {
class Basic {
  public:
    Basic(std::span<const MyCluster> clusters);
    uint32_t interval_count(const MyCluster *first, uint8_t size) const;
    uint32_t interval_sum(const MyCluster *first, uint8_t size) const;

  protected:
    const std::span<const MyCluster> clusters;
};
}; // namespace sumators
}; // namespace hist

#endif
