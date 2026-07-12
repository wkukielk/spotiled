#ifndef ANALYZERS_NON_LINEAR_HPP
#define ANALYZERS_NON_LINEAR_HPP

#include "alpha_coefficient.hpp"
#include "histogram_aux.hpp"
#include "sumators/non_linear.hpp"
#include "wrap_around.hpp"

namespace hist {
namespace analyzers {
namespace _nonlinear {
struct MaxCluster {
    const MyCluster *cluster;
    const uint32_t count;
    const uint8_t depth;
};
} // namespace _nonlinear
class NonLinear {
  public:
    NonLinear(const Histogram &, const sumators::NonLinear &sumator,
              uint8_t alpha = 1);
    uint8_t get_alpha() const { return alpha48.get_alpha(); };
    void set_alpha(uint8_t alpha) { alpha48.set_alpha(alpha); };
    Dominant get_dominant() const;

  protected:
    _nonlinear::MaxCluster get_max_cluster() const;
    uint16_t get_lower_bound(const _nonlinear::MaxCluster &cluster) const;
    uint16_t get_upper_bound(const _nonlinear::MaxCluster &cluster) const;
    uint32_t get_boundary_count() const;
    uint16_t get_representant_value(const MyCluster *, uint8_t depth) const;
    const Histogram &histogram;
    const sumators::NonLinear &sumator;
    const ClusterArray &clusters;
    AlphaCoefficient48 alpha48;
};
}; // namespace analyzers
}; // namespace hist

#endif
