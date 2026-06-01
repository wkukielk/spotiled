#include "MockDftEventLoop.hpp"
#include "MockLED.hpp"
#include "ledaux.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace testing;

class ChannelTest : public testing::Test {
  protected:
    void SetUp() override { esp_event_loop_create_default(); }
    void TearDown() override { esp_event_loop_delete_default(); }

    const ledc_channel_t channel = led::PWM::channel::RED;
    const gpio_num_t pin = led::pin::RED;
    MockLED &led_{MockLED::reset_and_get()};
};

TEST_F(ChannelTest, fade) {
    auto ch = led::Channel<MockLED, MockDftEventLoop>{channel, pin};
    auto tgt_duty = 100;
    EXPECT_CALL(led_, set_fade_time_and_start(_, channel, tgt_duty, _, _))
        .WillOnce(Return(ESP_OK));
    ASSERT_EQ(ESP_OK, ch.fade(tgt_duty));
}

TEST_F(ChannelTest, getDutySuccess) {
    auto ch = led::Channel<MockLED, MockDftEventLoop>{channel, pin};
    EXPECT_CALL(led_, get_duty(led::PWM::MODE, channel, _))
        .WillOnce(DoAll(SetArgReferee<2>(100), Return(ESP_OK)));
    ASSERT_EQ(100, ch.get_duty());
}

TEST_F(ChannelTest, getDutyFail) {
    auto ch = led::Channel<MockLED, MockDftEventLoop>{channel, pin};
    EXPECT_CALL(led_, get_duty(led::PWM::MODE, channel, _))
        .WillOnce(DoAll(SetArgReferee<2>(100), Return(ESP_FAIL)));
    ASSERT_THROW(ch.get_duty(), RuntimeError);
}

TEST_F(ChannelTest, registerCallbackSuccess) {
    auto ch = led::Channel<MockLED, MockDftEventLoop>{channel, pin};
    EXPECT_CALL(led_, cb_register(_, channel, _, _)).WillOnce(Return(ESP_OK));
    ch.register_callback();
}

TEST_F(ChannelTest, registerCallbackFail) {
    auto ch = led::Channel<MockLED, MockDftEventLoop>{channel, pin};
    EXPECT_CALL(led_, cb_register(_, channel, _, _)).WillOnce(Return(ESP_FAIL));
    ASSERT_THROW(ch.register_callback(), InitError);
}
