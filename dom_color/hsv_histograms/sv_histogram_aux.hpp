#ifndef SV_HISTOGRAM_AUX
#define SV_HISTOGRAM_AUX

#include "histogram2D_aux.hpp"
#include <cstddef>

namespace sv_hist {
struct SV {
    uint16_t s;
    uint16_t v;
};

struct SVBounds {
    hist2D::Bounds s;
    hist2D::Bounds v;
};

struct Dominant {
    SV value;
    SVBounds bounds;
    size_t count;
};

struct Index {
    uint8_t s;
    uint8_t v;
};
}; // namespace sv_hist

#endif
