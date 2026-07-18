#ifndef MOCK_LEDC_HOLDER_HPP
#define MOCK_LEDC_HOLDER_HPP

#include "MockLEDC.hpp"

namespace detail {
// Base-from-member idiom: bases are constructed before derived-class members,
// so a class that needs to bind a reference to a MockLEDC in its own base
// class initializer must hold that MockLEDC in a base of its own, listed
// before that base.
struct MockLEDCHolder {
    MockLEDC mock_ledc{};
};
} // namespace detail

#endif
