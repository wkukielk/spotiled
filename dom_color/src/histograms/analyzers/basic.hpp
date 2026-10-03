#ifndef ANALYZERS_BASIC_HPP
#define ANALYZERS_BASIC_HPP

#include "alpha_coefficient.hpp"
#include "histogram.hpp"
#include "histogram_aux.hpp"

namespace hist {
namespace analyzers {
class Basic {
  public:
    Basic(const Histogram &histogram, uint8_t alpha = 1);
    Dominant get_dominant() const;
    uint8_t get_alpha() const { return alpha6.get_alpha(); };
    void set_alpha(uint8_t alpha) { alpha6.set_alpha(alpha); };

  protected:
    const Histogram &histogram;
    AlphaCoefficient6 alpha6;
};
}; // namespace analyzers
}; // namespace hist

#endif
