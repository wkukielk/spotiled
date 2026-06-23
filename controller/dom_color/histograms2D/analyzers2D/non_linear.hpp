#ifndef ANALYZER2D_NON_LINEAR_HPP
#define ANALYZER2D_NON_LINEAR_HPP

#include "histogram2D.hpp"
#include "histogram2D_aux.hpp"
#include "sumators2D/non_linear.hpp"

namespace hist2D {
namespace analyzers {
namespace _nonlinear {
struct MaxCluster {
    const MyCluster *cluster;
    const uint32_t count;
    const uint8_t depth;
};
}; // namespace _nonlinear
class NonLinear {
  public:
    NonLinear(const Histogram &, const sumators::NonLinear &sumator);
    Dominant get_dominant() const;

  protected:
    _nonlinear::MaxCluster get_max_cluster() const;
    uint32_t get_boundary_count() const;
    Value get_representant_value(const MyCluster *center, uint8_t depth) const;
    ValueBounds get_cluster_bounds(const MyCluster *center,
                                   uint8_t depth) const;
    const Histogram &histogram;
    const sumators::NonLinear &sumator;
    const MyCluster (*const clusters)[sz][sz];
};
}; // namespace analyzers
}; // namespace hist2D

#endif
