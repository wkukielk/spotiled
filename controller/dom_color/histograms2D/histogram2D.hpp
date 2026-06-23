#ifndef HISTOGRAM2D_HPP
#define HISTOGRAM2D_HPP

#include "histogram2D_aux.hpp"

namespace hist2D {
enum AddError : uint16_t {
    TooManyElements = UINT16_MAX,
    OutOfRange = UINT16_MAX - 1
};

class Histogram {
  public:
    Histogram(uint16_t exclusive_upper_bound_x,
              uint16_t exclusive_upper_bound_y);
    ~Histogram();
    uint32_t get_count() const { return count; };
    uint16_t get_upper_bound_x() const { return upper_bound_x; };
    uint16_t get_upper_bound_y() const { return upper_bound_y; };
    const MyCluster (*get_clusters() const)[sz][sz];
    Index add(Value);
    Index add(Value, uint8_t weight);
    void reset();
    Index get_index(const MyCluster *) const;

  private:
    MyCluster (*const clusters)[sz][sz];
    uint32_t count = 0;
    uint16_t upper_bound_x = 0;
    uint16_t upper_bound_y = 0;
};
}; // namespace hist2D

#endif
