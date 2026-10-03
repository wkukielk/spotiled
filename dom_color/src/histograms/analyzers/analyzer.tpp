#ifndef ANALYZERS_ANALYZER_TPP
#define ANALYZERS_ANALYZER_TPP

#include "analyzer.hpp"

namespace hist {
namespace analyzers {
template <SearchAlgorithm Search>
Analyzer<Search>::Analyzer(const Histogram &histogram, uint8_t alpha)
    : histogram{histogram}, alpha{alpha} {}

template <SearchAlgorithm Search>
Dominant Analyzer<Search>::get_dominant() const {
    auto search = Search{histogram};
    while (auto candidate = search.get_next())
        if (alpha.is_count_enough(candidate->dominant.count,
                                  candidate->min_count, candidate->max_count))
            return candidate->dominant;
    return {};
}
}; // namespace analyzers
}; // namespace hist

#endif
