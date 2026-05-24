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
    q.send(&pi);
    int *out{};
    q.receive(&out);
    EXPECT_EQ(*out, i);
}

TEST_F(QueueTest, fifo) {
    util::Queue<int> q{4};
    int i10 = 10, i20 = 20;
    q.send(&i10);
    q.send(&i20);
    int out = 0;
    q.receive(&out);
    EXPECT_EQ(out, 10);
    q.receive(&out);
    EXPECT_EQ(out, 20);
}

TEST_F(QueueTest, sendFront) {
    util::Queue<int> q{4};
    int i10 = 10, i20 = 20;
    q.send(&i10);
    q.send_front(&i20);
    int out = 0;
    q.receive(&out);
    EXPECT_EQ(out, 20);
    q.receive(&out);
    EXPECT_EQ(out, 10);
}
