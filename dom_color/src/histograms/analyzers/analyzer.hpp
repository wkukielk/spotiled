#ifndef ANALYZERS_ANALYZER_HPP
#define ANALYZERS_ANALYZER_HPP

#include "alpha_coefficient.hpp"
#include "histogram.hpp"
#include "histogram_aux.hpp"
#include "search_algorithm.hpp"

namespace hist {
namespace analyzers {
// Returns the first candidate of the search whose count alpha accepts.
template <SearchAlgorithm Search> class Analyzer {
  public:
    Analyzer(const Histogram &histogram, uint8_t alpha = 1);
    uint8_t get_alpha() const { return alpha.get_alpha(); };
    void set_alpha(uint8_t alpha) { this->alpha.set_alpha(alpha); };
    Dominant get_dominant() const;

  private:
    const Histogram &histogram;
    AlphaCoefficient alpha;
};
}; // namespace analyzers
}; // namespace hist

#include "analyzer.tpp"

#endif
