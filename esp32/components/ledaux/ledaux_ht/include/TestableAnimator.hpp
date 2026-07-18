#ifndef TESTABLE_ANIMATOR_HPP
#define TESTABLE_ANIMATOR_HPP

#include "MockLEDC.hpp"
#include "animator.hpp"

class TestableAnimator : public Animator<MockLEDC> {
  public:
    MockLEDC &get_ledc() { return ledc; }
};

#endif
