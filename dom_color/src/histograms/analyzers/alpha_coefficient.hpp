#ifndef ALPHA_COEFFICIENT
#define ALPHA_COEFFICIENT

#include <cstddef>
#include <cstdint>

namespace hist {
namespace analyzers {
// alpha = 1 means "about 1/6 of total is enough"
// alpha = 255 means "only all of total is enough"
class AlphaCoefficient6 {
  public:
    AlphaCoefficient6(uint8_t alpha);
    uint8_t get_alpha() const { return alpha; };
    void set_alpha(uint8_t alpha);
    bool is_count_enough(uint32_t count, uint32_t total) const;

  private:
    uint8_t alpha;
    uint32_t alpha6;
    static const uint8_t alpha_shift = 8;
};

// alpha = 1 means "above boundary_count is enough"
// alpha = 255 means "only 48 times boundary_count is enough"
class AlphaCoefficient48 {
  public:
    AlphaCoefficient48(uint8_t alpha);
    uint8_t get_alpha() const { return alpha; };
    void set_alpha(uint8_t alpha);
    bool is_count_enough(uint32_t count, uint32_t boundary_count) const;

  private:
    uint8_t alpha;
    uint32_t alpha48;
    static const uint8_t alpha_shift = 4;
};
}; // namespace analyzers
}; // namespace hist

#endif
