#ifndef SUMATORS_NON_LINEAR_HPP
#define SUMATORS_NON_LINEAR_HPP

#include "histogram_aux.hpp"
#include "wrap_around.hpp"

namespace hist {
namespace sumators {
class NonLinear : public WrapAround {
  public:
    using WrapAround::WrapAround;
    uint64_t neighbourhood_count(const MyCluster *, uint8_t width) const;
    uint64_t neighbourhood_sum(const MyCluster *, uint8_t width) const;
    static uint8_t get_magic_number() { return magic_number; };
    static const uint8_t weights[];

  protected:
    static const uint8_t offsets[];
    static const uint8_t magic_number = 88;
};
}; // namespace sumators
}; // namespace hist

#endif
