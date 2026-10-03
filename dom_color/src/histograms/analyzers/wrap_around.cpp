#include "wrap_around.hpp"
#include "search_algorithm.hpp"

namespace hist {
namespace analyzers {
WrapAround::WrapAround(const Histogram &histogram, uint8_t alpha)
    : histogram{histogram}, alpha6{alpha} {}

Dominant WrapAround::get_dominant() const {
    auto search = WrappingSearchAlgorithm{histogram};
    while (auto dominant = search.get_next())
        if (alpha6.is_count_enough(dominant->count, histogram.get_count()))
            return *dominant;
    return {};
}
}; // namespace analyzers
}; // namespace hist
