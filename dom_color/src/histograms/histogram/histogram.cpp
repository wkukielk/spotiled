#include "histogram.hpp"
#include <algorithm>
#include <cmath>

namespace hist {
namespace {
struct MaxCluster {
    const MyCluster *cluster = nullptr;
    uint32_t count = 0;
    uint8_t depth = 0;
};
} // namespace

Histogram::Histogram(uint16_t exclusive_upper_bound)
    : clusters(sz), upper_bound{exclusive_upper_bound} {}

uint16_t Histogram::add(uint16_t value) { return add(value, 1); }

uint16_t Histogram::add(uint16_t value, uint8_t weight) {
    if (count + weight > 1 << 20)
        return AddError::TooManyElements;
    if (value >= upper_bound)
        return AddError::OutOfRange;

    uint16_t index = (value * sz) / upper_bound;
    clusters[index].add(value, weight);
    count += weight;
    return index;
}

void Histogram::reset() {
    count = 0;
    std::fill(clusters.begin(), clusters.end(), MyCluster{});
}
}; // namespace hist
