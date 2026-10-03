#ifndef HISTOGRAM_ANALYZER_HPP
#define HISTOGRAM_ANALYZER_HPP

#include "histogram.hpp"
#include "histogram_aux.hpp"
#include "sumators/common.hpp"

template <hist::Sumator S> class HistogramSumator {
  public:
    HistogramSumator(const hist::Histogram &histogram, S &&sumator);
    uint32_t count(S::window_t w) const { return sumator.count(w); }
    hist::Dominant window_to_dominant(S::window_t w) const;

  private:
    uint16_t get_representant_value(S::window_t w) const;

    S sumator;
    const hist::Histogram &histogram;
};

#include "histogram_sumator.tpp"

#endif
