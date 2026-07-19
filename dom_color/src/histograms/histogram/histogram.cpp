#include "histogram.hpp"
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
    : upper_bound{exclusive_upper_bound} {}

uint16_t Histogram::add(uint16_t value) {
    if (count > (1 << 20) - 1)
        return AddError::TooManyElements;
    if (value >= upper_bound) {
        return AddError::OutOfRange;
    }
    uint16_t index = value * sz / upper_bound;
    clusters.at(index).add(value);
    ++count;
    return index;
}

uint16_t Histogram::add(uint16_t value, uint8_t weight) {
    if (count > (uint32_t)((1 << 20) - weight))
        return AddError::TooManyElements;
    if (value >= upper_bound) {
        return AddError::OutOfRange;
    }
    uint16_t index = (value * sz) / upper_bound;
    clusters[index].add(value, weight);
    count += weight;
    return index;
}

void Histogram::reset() {
    count = 0;
    clusters.fill(MyCluster{});
}
}; // namespace hist
