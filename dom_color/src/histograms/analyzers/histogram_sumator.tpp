#ifndef HISTOGRAM_ANALYZER_HPP
#include "histogram_sumator.hpp"
#endif

template <hist::Sumator S>
HistogramSumator<S>::HistogramSumator(const hist::Histogram &histogram,
                                      S &&sumator)
    : sumator{std::move(sumator)}, histogram{histogram} {}

template <hist::Sumator S>
uint16_t HistogramSumator<S>::get_representant_value(S::window_t w) const {
    uint64_t sum = sumator.sum(w);
    uint32_t count = sumator.count(w);
    uint64_t boundary =
        (uint64_t)count * (histogram.get_upper_bound() >> hist::hshift);
    if (sum >= boundary)
        sum -= boundary;
    return count != 0 ? (sum << hist::hshift) / count : 0;
}

template <hist::Sumator S>
hist::Dominant HistogramSumator<S>::window_to_dominant(S::window_t w) const {
    return {.bounds = get_bounds(histogram, w),
            .value = get_representant_value(w),
            .count = count(w)};
}
