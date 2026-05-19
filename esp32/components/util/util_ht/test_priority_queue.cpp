#include "queue.hpp"
#include <gtest/gtest.h>

class QueueTest : public testing::Test {
  protected:
    QueueTest() {}
};

TEST_F(QueueTest, decodeSuccess) {
    util::Queue<int *> q{4};
    int i = 42;
    int *pi = &i;
    q.send(&pi, 0);
    int *out{};
    q.receive(&out);
    EXPECT_EQ(*out, i);
}
