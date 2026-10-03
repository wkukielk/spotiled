#ifndef ALPHA_COEFFICIENT
#define ALPHA_COEFFICIENT

#include <cstddef>
#include <cstdint>

namespace hist {
namespace analyzers {
// Interpolates linearly between a count the search is guaranteed to reach
// and the highest count it could reach:
// alpha = 1 means "the guaranteed count is enough"
// alpha = 255 means "only the maximum count is enough"
// alpha = 0 sits just below the guaranteed count.
class AlphaCoefficient {
  public:
    AlphaCoefficient(uint8_t alpha) : alpha{alpha} {}
    uint8_t get_alpha() const { return alpha; };
    void set_alpha(uint8_t alpha) { this->alpha = alpha; };
    bool is_count_enough(uint64_t count, uint64_t min_count,
                         uint64_t max_count) const;

  private:
    uint8_t alpha;
};
}; // namespace analyzers
}; // namespace hist

#endif
