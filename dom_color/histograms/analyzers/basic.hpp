#ifndef ANALYZERS_BASIC_HPP
#define ANALYZERS_BASIC_HPP

#include "alpha_coefficient.hpp"
#include "histogram.hpp"
#include "histogram_aux.hpp"
#include "sumators/basic.hpp"

namespace hist {
namespace analyzers {
namespace _basic {
struct MaxCluster {
    const MyCluster *cluster;
    const uint32_t count;
    const uint8_t size;
};
}; // namespace _basic
class Basic {
  public:
    Basic(const Histogram &histogram, const sumators::Basic &sumator,
          uint8_t alpha = 1);
    Dominant get_dominant() const;
    uint8_t get_alpha() const { return alpha6.get_alpha(); };
    void set_alpha(uint8_t alpha) { alpha6.set_alpha(alpha); };

  protected:
    _basic::MaxCluster get_max_cluster() const;
    uint16_t get_representant_value(const MyCluster *, uint8_t length) const;
    const Histogram &histogram;
    const sumators::Basic &sumator;
    const ClusterArray &clusters;
    AlphaCoefficient6 alpha6;
};
}; // namespace analyzers
}; // namespace hist

#endif
