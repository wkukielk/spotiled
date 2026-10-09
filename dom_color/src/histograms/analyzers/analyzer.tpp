#ifndef ANALYZERS_ANALYZER_TPP
#define ANALYZERS_ANALYZER_TPP

#include "analyzer.hpp"

namespace hist {
namespace analyzers {
template <SearchAlgorithm Search>
Analyzer<Search>::Analyzer(const Histogram &histogram, uint8_t alpha)
    : histogram{histogram},
      sumator{histogram.get_clusters(), histogram.get_upper_bound()},
      alpha{alpha} {}

template <SearchAlgorithm Search>
BasicWindow Analyzer<Search>::to_window(Bounds bounds) const {
    auto upper_bound = histogram.get_upper_bound();
    auto to_index = [upper_bound](uint32_t value) {
        return (value * sz + upper_bound - 1) / upper_bound;
    };
    size_t first = to_index(bounds.lower) % sz;
    size_t size = (to_index(bounds.upper) + sz - first) % sz;
    return {histogram.get_clusters().data() + first, (uint8_t)size};
}

template <SearchAlgorithm Search>
Dominant Analyzer<Search>::get_dominant() const {
    auto search = Search{histogram};
    while (auto dominant = search.get_next())
        if (alpha.is_count_enough(sumator.count(to_window(dominant->bounds)), 0,
                                  histogram.get_count()))
            return *dominant;
    return {};
}
}; // namespace analyzers
}; // namespace hist

#endif
