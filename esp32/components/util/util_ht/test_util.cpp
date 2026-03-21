#include "BaseTest.hpp"
#include "MockEventGroup.hpp"
#include "MockTime.hpp"
#include "time_aux.hpp"
#include "util.hpp"
#include <gtest/gtest.h>

using namespace util;
using namespace std;

class ThreadTest : public BaseTest {
  protected:
    ThreadTest()
        : egroup{mkuptr<MockEventGroup>()}, real{mkuptr<EventGroup>()} {
        params = thread_params{.name = "test", .join_bit = join_bit};
    }
    ~ThreadTest() {}

    EventBits_t join_bit = 1 << 0;
    u_ptr<MockEventGroup> egroup{};
    u_ptr<EventGroup> real{};
    thread_params params;

    MockTime timer{};

    static void stub(void *params);
};

void ThreadTest::stub(void *params) {
    auto &test = *(ThreadTest *)params;
    test.timer.delay_ms(1000);
};

TEST_F(ThreadTest, join_real) {
    params.fun = ThreadTest::stub;
    params.fun_parameters = this;
    params.join_group = real.get();
    EXPECT_CALL(timer, delay_ms(1000));
    auto thread = mkuptr<Thread>(move(params));
    thread->join();
    time_aux_delay_ms(50);
}

TEST_F(ThreadTest, join) {
    params.fun = ThreadTest::stub;
    params.fun_parameters = this;
    params.join_group = egroup.get();
    EXPECT_CALL(timer, delay_ms(1000));
    EXPECT_CALL(*egroup, set(join_bit));
    auto thread = mkuptr<Thread>(move(params));
    EXPECT_CALL(*egroup, wait(join_bit));
    thread->join();
    time_aux_delay_ms(50);
}
