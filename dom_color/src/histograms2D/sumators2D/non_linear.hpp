#pragma once

#include "common.hpp"
#include "histogram2D.hpp"

namespace hist2D {
namespace sumators {
struct Sum64 {
    uint64_t x;
    uint64_t y;
};

class NonLinear {
  public:
    using window_t = CenterWindow;

    NonLinear(const Histogram &);
    uint64_t count(CenterWindow w) const;
    Sum64 sum(CenterWindow w) const;
    static const uint8_t weights[5][5];

  protected:
    const Histogram &histogram;
};
}; // namespace sumators
}; // namespace hist2D
