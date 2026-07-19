#ifndef HISTOGRAM_AUX
#define HISTOGRAM_AUX

#include "cluster.hpp"
#include <array>
#include <cstddef>

namespace hist {
static const uint8_t basesz = 6;
static const uint8_t halvings = 5;
static const size_t sz = basesz * (1 << halvings);
static const uint8_t hshift = 4;
using MyCluster = Cluster<hshift>;
using ClusterArray = std::array<MyCluster, sz>;

struct Bounds {
    uint16_t lower;
    uint16_t upper;
};

struct Dominant {
    Bounds bounds;
    uint16_t value;
    uint32_t count;
};

class Precission {
  public:
    Precission(uint8_t starting_precission);
    uint8_t get_precission() const { return precission; };
    size_t get_cluster_size() const { return cluster_size; };
    Precission &operator++();
    Precission &operator--();

  private:
    uint8_t precission;
    size_t cluster_size = 1;
};
}; // namespace hist

#endif
