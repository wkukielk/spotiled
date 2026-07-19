#ifndef ANALYZERS_WRAP_AROUND_HPP
#define ANALYZERS_WRAP_AROUND_HPP

#include "alpha_coefficient.hpp"
#include "basic.hpp"
#include "histogram_aux.hpp"
#include "sumators/wrap_around.hpp"

namespace hist {
namespace analyzers {
namespace _wraparound {
struct MaxCluster {
    const MyCluster *cluster;
    const uint32_t count;
    const uint8_t size;
};
}; // namespace _wraparound
class WrapAround {
  public:
    WrapAround(const Histogram &clusters, const sumators::WrapAround &sumator,
               uint8_t alpha = 1);
    uint8_t get_alpha() const { return alpha6.get_alpha(); };
    void set_alpha(uint8_t alpha) { alpha6.set_alpha(alpha); };
    Dominant get_dominant() const;

  protected:
    _wraparound::MaxCluster get_max_cluster() const;
    uint16_t get_representant_value(const MyCluster *, uint8_t length) const;

    const Histogram &histogram;
    const sumators::WrapAround &sumator;
    const ClusterArray &clusters;
    AlphaCoefficient6 alpha6;
};
}; // namespace analyzers
}; // namespace hist

#endif
