#include "queue.hpp"
#include <gtest/gtest.h>

class PriorityQueueTest : public testing::Test {
  protected:
    PriorityQueueTest() {}
};

TEST_F(PriorityQueueTest, decodeSuccess) {
    util::PriorityQueue q{4, sizeof(int *), 3};
    int i = 42;
    int *pi = &i;
    q.send(&pi, 0);
    int *out{};
    q.receive(&out);
    EXPECT_EQ(*out, i);
}
