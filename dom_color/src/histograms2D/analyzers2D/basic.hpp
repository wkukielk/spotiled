#ifndef ANALYZER2D_BASIC
#define ANALYZER2D_BASIC

#include "analyzers/alpha_coefficient.hpp"
#include "histogram2D.hpp"
#include "histogram2D_aux.hpp"
#include "sumators2D/basic.hpp"

namespace hist2D {
namespace analyzers {
class Basic {
  public:
    Basic(const Histogram &histogram, const sumators::Basic &sumator,
          uint8_t alpha = 1);
    uint8_t get_alpha() const { return alpha.get_alpha(); };
    void set_alpha(uint8_t alpha) { this->alpha.set_alpha(alpha); };
    Dominant get_dominant() const;

  protected:
    const Histogram &histogram;
    const sumators::Basic &sumator;
    hist::analyzers::AlphaCoefficient alpha;
};
}; // namespace analyzers
}; // namespace hist2D

#endif
