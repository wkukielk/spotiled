#ifndef HISTOGRAM_HPP
#define HISTOGRAM_HPP

#include "histogram_aux.hpp"
#include <array>

namespace hist {
enum AddError : uint16_t {
    TooManyElements = UINT16_MAX,
    OutOfRange = UINT16_MAX - 1
};

class Histogram {
  public:
    Histogram(uint16_t exclusive_upper_bound);
    uint32_t get_count() const { return count; };
    uint16_t get_upper_bound() const { return upper_bound; };
    const ClusterArray &get_clusters() const { return clusters; };
    uint16_t add(uint16_t value);
    uint16_t add(uint16_t value, uint8_t weight);
    void reset();

  private:
    ClusterArray clusters;
    uint32_t count = 0;
    uint16_t upper_bound;
};
}; // namespace hist

#endif
