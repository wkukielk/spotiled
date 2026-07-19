#include "v_cluster.hpp"


void VCluster::add(const HSV& hsv) {
    v_sum += hsv.v;
    ++count;
}

uint16_t VCluster::get_representant() const {
    return count != 0 ? v_sum / count : 0;
}

size_t VCluster::get_count() const { return count; }

void VCluster::reset() {
    v_sum = 0;
    count = 0;
}
