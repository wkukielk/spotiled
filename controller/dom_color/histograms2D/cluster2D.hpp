#ifndef CLUSTER2D_HPP
#define CLUSTER2D_HPP

#include <cstddef>
#include <cstdint>

struct Value {
    uint16_t x;
    uint16_t y;
};

template <uint8_t shift = 0> struct Cluster2D {
    uint32_t sum_x = 0;
    uint32_t sum_y = 0;
    size_t count = 0;
    void add(Value);
    void add(Value, uint8_t weight);
    void reset();
};

template <uint8_t shift> void Cluster2D<shift>::add(Value value) {
    ++count;
    sum_x += value.x >> shift;
    sum_y += value.y >> shift;
}

template <uint8_t shift>
void Cluster2D<shift>::add(Value value, uint8_t weight) {
    count += weight;
    sum_x += weight * (value.x >> shift);
    sum_y += weight * (value.y >> shift);
}

template <uint8_t shift> void Cluster2D<shift>::reset() {
    sum_x = 0;
    sum_y = 0;
    count = 0;
}

#endif
