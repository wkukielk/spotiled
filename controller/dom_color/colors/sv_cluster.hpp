#ifndef SV_CLUSTER_HPP
#define SV_CLUSTER_HPP

#include "hsv.hpp"

template <uint8_t shift = 0> struct SVCluster {
    uint32_t s_sum = 0;
    uint32_t v_sum = 0;
    size_t count = 0;
    void add(const HSV &);
    void add(const HSV &, uint8_t weight);
    void reset();
};

template <uint8_t shift> void SVCluster<shift>::add(const HSV &hsv) {
    ++count;
    s_sum += hsv.s >> shift;
    v_sum += hsv.v >> shift;
}

template <uint8_t shift>
void SVCluster<shift>::add(const HSV &hsv, uint8_t weight) {
    count += weight;
    s_sum += weight * (hsv.s >> shift);
    v_sum += weight * (hsv.v >> shift);
}

template <uint8_t shift> void SVCluster<shift>::reset() {
    s_sum = 0;
    v_sum = 0;
    count = 0;
}

#endif
