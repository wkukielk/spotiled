#ifndef ANALYZER2D_NON_LINEAR_HPP
#define ANALYZER2D_NON_LINEAR_HPP

#include "analyzers/alpha_coefficient.hpp"
#include "histogram2D.hpp"
#include "histogram2D_aux.hpp"
#include "sumators2D/non_linear.hpp"

namespace hist2D {
namespace analyzers {
class NonLinear {
  public:
    NonLinear(const Histogram &, const sumators::NonLinear &sumator,
              uint8_t alpha = 1);
    uint8_t get_alpha() const { return alpha.get_alpha(); };
    void set_alpha(uint8_t alpha) { this->alpha.set_alpha(alpha); };
    Dominant get_dominant() const;

  protected:
    const Histogram &histogram;
    const sumators::NonLinear &sumator;
    hist::analyzers::AlphaCoefficient alpha;
};
}; // namespace analyzers
}; // namespace hist2D

#endif
