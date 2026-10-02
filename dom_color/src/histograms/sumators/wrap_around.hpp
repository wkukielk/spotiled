#ifndef SUMATORS_WRAP_AROUND
#define SUMATORS_WRAP_AROUND

#include "histogram_aux.hpp"
#include <span>

namespace hist {
namespace sumators {
class WrapAround {
  public:
    WrapAround(std::span<const MyCluster>, uint16_t exclusive_upper_bound);
    uint32_t wrap_interval_count(const MyCluster *clusters, size_t size) const;
    uint64_t wrap_interval_sum(const MyCluster *, size_t length) const;

  protected:
    const std::span<const MyCluster> clusters;
    const uint16_t upper_bound;
};
}; // namespace sumators
}; // namespace hist

#endif
