#ifndef ANALYZERS_NON_LINEAR_HPP
#define ANALYZERS_NON_LINEAR_HPP

#include "alpha_coefficient.hpp"
#include "histogram_aux.hpp"
#include "sumators/non_linear.hpp"
#include "wrap_around.hpp"

namespace hist {
namespace analyzers {
class NonLinear {
  public:
    NonLinear(const Histogram &, uint8_t alpha = 1);
    uint8_t get_alpha() const { return alpha48.get_alpha(); };
    void set_alpha(uint8_t alpha) { alpha48.set_alpha(alpha); };
    Dominant get_dominant() const;

  protected:
    uint32_t get_boundary_count() const;
    const Histogram &histogram;
    AlphaCoefficient48 alpha48;
};
}; // namespace analyzers
}; // namespace hist

#endif
