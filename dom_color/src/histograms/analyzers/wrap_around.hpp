#ifndef ANALYZERS_WRAP_AROUND_HPP
#define ANALYZERS_WRAP_AROUND_HPP

#include "alpha_coefficient.hpp"
#include "histogram.hpp"
#include "histogram_aux.hpp"

namespace hist {
namespace analyzers {
class WrapAround {
  public:
    WrapAround(const Histogram &histogram, uint8_t alpha = 1);
    uint8_t get_alpha() const { return alpha6.get_alpha(); };
    void set_alpha(uint8_t alpha) { alpha6.set_alpha(alpha); };
    Dominant get_dominant() const;

  protected:
    const Histogram &histogram;
    AlphaCoefficient6 alpha6;
};
}; // namespace analyzers
}; // namespace hist

#endif
