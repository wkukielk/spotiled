#ifndef SUMATORS2D_BASIC
#define SUMATORS2D_BASIC

#include "histogram2D_aux.hpp"

namespace hist2D {
namespace sumators {
class Basic {
  public:
    Basic(const MyCluster (*clusters)[sz][sz]);
    uint32_t interval_count(const MyCluster *topleft,
                            uint8_t square_size) const;
    Sum interval_sum(const MyCluster *topleft, uint8_t square_size) const;

  protected:
    const MyCluster *const begin;
    const MyCluster *const end;
};
}; // namespace sumators
}; // namespace hist2D

#endif
