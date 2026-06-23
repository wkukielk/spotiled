#ifndef HISTOGRAM2D_AUX_HPP
#define HISTOGRAM2D_AUX_HPP

#include "cluster2D.hpp"
#include <cstddef>
#include <cstdint>

namespace hist2D {
static const uint8_t basesz = 3;
static const uint8_t halvings = 2;
static const size_t sz = basesz * (1 << halvings);
static const uint8_t hshift = 4;
using MyCluster = Cluster2D<hshift>;

struct Bounds {
    uint16_t lower;
    uint16_t upper;
};

struct ValueBounds {
    Bounds x;
    Bounds y;
};

struct Dominant {
    Value value;
    ValueBounds bounds;
    size_t count;
};

struct Index {
    uint8_t x;
    uint8_t y;
};

struct Sum {
    uint32_t x;
    uint32_t y;
};
}; // namespace hist2D

#endif
