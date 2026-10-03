#ifndef ANALYZER2D_BASIC
#define ANALYZER2D_BASIC

#include "histogram2D.hpp"
#include "histogram2D_aux.hpp"
#include "sumators2D/basic.hpp"

namespace hist2D {
namespace analyzers {
struct _MaxCluster {
    const MyCluster *topleft;
    uint32_t count;
    uint8_t square_size;
};

class Basic {
  public:
    Basic(const Histogram &histogram, const sumators::Basic &sumator);
    Dominant get_dominant() const;

  protected:
    _MaxCluster get_max_cluster() const;
    Value get_representant_value(const MyCluster *topleft, uint8_t size) const;
    const Histogram &histogram;
    const sumators::Basic &sumator;
    const MyCluster (*const clusters)[sz][sz];
};
}; // namespace analyzers
}; // namespace hist2D

#endif
