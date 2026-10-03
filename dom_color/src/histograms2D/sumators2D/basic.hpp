#ifndef SUMATORS2D_BASIC
#define SUMATORS2D_BASIC

#include "common.hpp"
#include "histogram2D_aux.hpp"

namespace hist2D {
namespace sumators {
class Basic {
  public:
    using window_t = BasicWindow;

    Basic(const MyCluster (*clusters)[sz][sz]);
    uint32_t count(BasicWindow w) const;
    Sum sum(BasicWindow w) const;

  protected:
    const MyCluster *const begin;
    const MyCluster *const end;
};
}; // namespace sumators
}; // namespace hist2D

#endif
