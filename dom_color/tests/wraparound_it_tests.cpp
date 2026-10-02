#include <gtest/gtest.h>

#include "wraparound_it.hpp"
#include <vector>

using namespace std;

TEST(WrapAroundIt, IncrementWraps) {
    auto arr = vector{1, 2, 3};
    auto it = wraparound_iterator{arr, arr.cbegin()};
    EXPECT_EQ(*it, 1);
    EXPECT_EQ(*(++it), 2);
    EXPECT_EQ(*(++it), 3);
    EXPECT_EQ(*(++it), 1);
}

TEST(WrapAroundIt, DecrementWraps) {
    auto arr = vector{1, 2, 3};
    auto it = wraparound_iterator{arr, arr.cbegin()};
    EXPECT_EQ(*(--it), 3);
    EXPECT_EQ(*(--it), 2);
}

TEST(WrapAroundIt, PostfixReturnsOldValue) {
    auto arr = vector{1, 2, 3};
    auto it = wraparound_iterator{arr, arr.cend() - 1};
    EXPECT_EQ(*(it++), 3);
    EXPECT_EQ(*it, 1);
    EXPECT_EQ(*(it--), 1);
    EXPECT_EQ(*it, 3);
}

TEST(WrapAroundIt, ArithmeticWrapsMoreThanOnce) {
    auto arr = vector{1, 2, 3};
    auto it = wraparound_iterator{arr, arr.cbegin()};
    EXPECT_EQ(*(it + 7), 2);
    EXPECT_EQ(*(it - 7), 3);
    EXPECT_TRUE(it + 3 == it);
}

TEST(WrapAroundIt, RejectsOutOfRangePointer) {
    auto arr = vector{1, 2, 3};
    EXPECT_THROW((wraparound_iterator{arr, arr.cend()}), std::runtime_error);
}
