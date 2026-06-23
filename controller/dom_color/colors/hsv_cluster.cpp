#include "hsv_cluster.hpp"


HSV HSVCluster::get_representant() const {
    auto representant = HSV{};
    representant.h = h_sum  / count;
    representant.s = s_sum / count;
    representant.v = v_sum / count;
    return representant;
}

void HSVCluster::add(const HSV& hsv) {
    ++count;
    s_sum += hsv.s;
    v_sum += hsv.v;
}
