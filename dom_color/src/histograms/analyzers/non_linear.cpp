#include "non_linear.hpp"
#include "search_algorithm.hpp"

namespace hist {
namespace analyzers {
NonLinear::NonLinear(const Histogram &histogram, uint8_t alpha)
    : histogram{histogram}, alpha48{alpha} {}

Dominant NonLinear::get_dominant() const {
    auto search = NonLinearSearchAlgorithm{histogram};
    while (auto dominant = search.get_next())
        if (alpha48.is_count_enough(dominant->count, get_boundary_count()))
            return *dominant;
    return {};
}

uint32_t NonLinear::get_boundary_count() const {
    return sumators::NonLinear::get_magic_number() * histogram.get_count() / sz;
}
}; // namespace analyzers
}; // namespace hist
