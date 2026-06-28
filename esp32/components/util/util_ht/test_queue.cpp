#include "queue.hpp"
#include "test_timing.hpp"
#include <gtest/gtest.h>

using namespace testing;

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

TEST_F(QueueTest, sendToFull) {
    util::Queue<int> q{1};
    int i10 = 10, i20 = 20;
    q.send(&i10);

    auto delay = pdMS_TO_TICKS(10);
    auto [ret, ms] = time_fun([&]() { return q.send(&i20, delay); });
    ASSERT_EQ(ret, ESP_FAIL);
    EXPECT_THAT(ms, in_range(5, 15));
}

TEST_F(QueueTest, receiveFromEmpty) {
    util::Queue<int> q{4};

    auto i = 42;
    auto delay = pdMS_TO_TICKS(10);
    auto [ret, ms] = time_fun([&]() { return q.receive(&i, delay); });
    ASSERT_EQ(ret, ESP_FAIL);
    ASSERT_EQ(i, 42);
    EXPECT_THAT(ms, in_range(7, 13));
}

TEST_F(QueueTest, noWait) {
    util::Queue<int> q{4};
    int i = 42;
    q.send(&i);
    int out = 0;
    ASSERT_EQ(q.receive_nowait(&out), ESP_OK);
    EXPECT_EQ(out, i);
}

TEST_F(QueueTest, receiveNoWaitEmpty) {
    util::Queue<int> q{4};
    int i = 42;
    ASSERT_EQ(q.receive_nowait(&i), ESP_FAIL);
    EXPECT_EQ(i, 42);
}
