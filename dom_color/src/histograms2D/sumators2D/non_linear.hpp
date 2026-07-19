#pragma once

#include "basic.hpp"
#include "histogram2D.hpp"

namespace hist2D {
namespace sumators {
struct Sum64 {
    uint64_t x;
    uint64_t y;
};

struct Offsets {
    int8_t lower;
    int8_t upper;
};

struct ClusterOffsets {
    Offsets x;
    Offsets y;
};

class NonLinear : public Basic {
  public:
    NonLinear(const Histogram &);
    uint64_t neighbourhood_count(const MyCluster *center, uint8_t depth) const;
    Sum64 neighbourhood_sum(const MyCluster *center, uint8_t depth) const;
    ClusterOffsets get_cluster_offsets(const MyCluster *center,
                                       uint8_t depth) const;
    static uint8_t get_magic_number() { return magic_number; };
    static const uint8_t weights[5][5];

  protected:
    const Histogram &histogram;
    static const uint8_t magic_number = 86;
};
}; // namespace sumators
}; // namespace hist2D
