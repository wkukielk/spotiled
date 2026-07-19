#ifndef CLUSTER_HPP
#define CLUSTER_HPP

#include <cstddef>
#include <cstdint>

template <uint8_t shift = 0> struct Cluster {
    uint32_t sum = 0;
    size_t count = 0;
    void add(uint16_t);
    void add(uint16_t, uint8_t weight);
    void reset();
};

template <uint8_t shift> void Cluster<shift>::add(uint16_t value) {
    ++count;
    sum += value >> shift;
}

template <uint8_t shift>
void Cluster<shift>::add(uint16_t value, uint8_t weight) {
    count += weight;
    sum += weight * (value >> shift);
}

template <uint8_t shift> void Cluster<shift>::reset() {
    sum = 0;
    count = 0;
}

#endif
