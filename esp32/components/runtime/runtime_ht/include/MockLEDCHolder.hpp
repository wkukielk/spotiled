#ifndef MOCK_LEDC_HOLDER_HPP
#define MOCK_LEDC_HOLDER_HPP

#include "MockLEDC.hpp"

namespace detail {
struct MockLEDCHolder {
    MockLEDC mock_ledc{};
};
} // namespace detail

#endif
