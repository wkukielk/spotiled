#include "basic.hpp"
#include "search_algorithm.hpp"

namespace hist {
namespace analyzers {
Basic::Basic(const Histogram &histogram, uint8_t alpha)
    : histogram{histogram}, alpha6{alpha} {}

Dominant Basic::get_dominant() const {
    auto search = BasicSearchAlgorithm{histogram};
    while (auto dominant = search.get_next())
        if (alpha6.is_count_enough(dominant->count, histogram.get_count()))
            return *dominant;
    return {};
}
}; // namespace analyzers
}; // namespace hist
