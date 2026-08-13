#include <gtest/gtest.h>

#include "rgb.hpp"

TEST(Smoke, RGBConstructsAndComparesEqual) {
    RGB a(1, 2, 3);
    RGB b(1, 2, 3);
    EXPECT_EQ(a, b);
}
