#include "bridge.hpp"

RGB get_dominating_color(rust::Slice<const uint16_t> pixels)
{
    return get_dominating_color(std::span<const uint16_t>{pixels.data(), pixels.size()});
}
