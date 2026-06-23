#include "histogram_aux.hpp"

namespace hist {
Precission::Precission(uint8_t starting_precission)
    : precission{starting_precission} {}

Precission &Precission::operator++() {
    ++precission;
    cluster_size /= 2;
    return *this;
}

Precission &Precission::operator--() {
    --precission;
    cluster_size *= 2;
    return *this;
}
}; // namespace hist
