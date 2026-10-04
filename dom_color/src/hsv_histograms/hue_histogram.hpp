#ifndef HUE_HISTOGRAM_HPP
#define HUE_HISTOGRAM_HPP

#include "analyzers/analyzer.hpp"
#include "histogram.hpp"
#include "hsv.hpp"
#include "sumators/non_linear.hpp"
#include "wraparound_it.hpp"
#include <array>
#include <cstddef>

namespace hue_hist {
struct Dominant {
    hist::Bounds bounds;
    uint16_t hue;
    uint32_t count;
    bool is_grey;
    static Dominant grey;
};

Dominant create_grey(uint32_t grey_count);
Dominant convert_dominant(const hist::Dominant &dominant);
}; // namespace hue_hist

class HueHistogram {
  public:
    HueHistogram(uint8_t alpha = 3, uint8_t beta = 23);
    uint32_t get_count() const { return histogram.get_count(); };
    uint32_t get_grey_count() const { return alpha * grey_count; }
    uint64_t get_score() const { return get_count(); };
    uint8_t get_alpha() const { return alpha; };
    void set_alpha(uint8_t alpha) { this->alpha = alpha; };
    uint8_t get_beta() const { return beta; };
    void set_beta(uint8_t beta) { this->beta = beta; };
    const hist::Histogram &get_histogram() const { return histogram; }
    hue_hist::Dominant get_dominant();
    uint16_t add_pixel(const HSV &);
    void reset();

  private:
    uint8_t calculate_beta() const;
    hist::Histogram histogram;
    hist::analyzers::Analyzer<WrappingSearchAlgorithm> analyzer;
    uint16_t grey_count = 0;
    uint8_t alpha = 1;
    // calculation of dominant starts if grey_count / count >= alpha
    uint8_t beta = 1; // passed to the analyzer as its "alpha coefficient"
};

#endif
