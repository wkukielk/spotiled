#ifndef V_CLUSTER_HPP
#define V_CLUSTER_HPP

#include "hsv.hpp"
#include <cstddef>

struct DominantV {
    uint16_t v;
    size_t count;
};

struct VCluster {
    uint32_t v_sum = 0;
    size_t count = 0;
    void add(const HSV &);
    void reset();
    uint16_t get_representant() const;
    size_t get_count() const;
};

#endif
