#ifndef HISTOGRAM_AUX
#define HISTOGRAM_AUX

#include "cluster.hpp"
#include <cstddef>
#include <vector>

namespace hist {
static const uint8_t basesz = 6;
static const uint8_t halvings = 5;
static const size_t sz = basesz * (1 << halvings);
static const uint8_t hshift = 4;
using MyCluster = Cluster<hshift>;
// Heap allocated; always holds exactly sz clusters.
using ClusterArray = std::vector<MyCluster>;

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
    bool operator==(const Precission &other) const noexcept;

  private:
    uint8_t precission;
    size_t cluster_size = 1;
};
}; // namespace hist

#endif
