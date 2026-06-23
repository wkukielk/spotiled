#ifndef HSV_CLUSTER_HPP
#define HSV_CLUSTER_HPP

#include "hsv.hpp"
#include <cstddef>

struct HSVCluster {
    uint32_t h_sum;
    uint32_t s_sum;
    uint32_t v_sum;
    size_t count; // nr of pixels that fall into this cluster
    HSV get_representant() const;
    void add(const HSV &);
};

#endif
