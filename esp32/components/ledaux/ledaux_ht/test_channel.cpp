#include "MockDftEventLoop.hpp"
#include "MockLED.hpp"
#include "ledaux.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <optional>

using namespace testing;

// Tests that exercise construction and destruction of Channel.
// No Channel is pre-built; each test controls the mock setup.
class ChannelConstructTest : public testing::Test {
  protected:
    using ELoop = Handle<MockDftEventLoop>;
    const ledc_channel_t channel = led::PWM::channel::RED;
    const gpio_num_t pin = led::pin::RED;

    ELoop dft_eloop{};
    MockDftEventLoop &eloop = dft_eloop.get();

    MockLED &led_{MockLED::reset_and_get()};
};

TEST_F(ChannelConstructTest, configFail) {
    EXPECT_CALL(led_, configure_channel(_)).WillOnce(Return(ESP_FAIL));
    ASSERT_THROW((led::Channel<MockLED, ELoop>{channel, pin}), InitError);
}

TEST_F(ChannelConstructTest, callbackRegisterFail) {
    EXPECT_CALL(led_, cb_register(_, channel, &led::fade_cb_s, _))
        .WillOnce(Return(ESP_FAIL));
    ASSERT_THROW((led::Channel<MockLED, ELoop>{channel, pin}), InitError);
}

// Tests that assume construction succeeded and exercise Channel methods.
// SetUp builds the channel; tests get a ready-to-use ch_.
class ChannelTest : public testing::Test {
  public:
    void SetUp() override {
        EXPECT_CALL(led_, cb_register(_, channel, &led::fade_cb_s, _))
            .WillOnce(
                DoAll(SaveArg<3, void **>((void **)&pen_), Return(ESP_OK)));
        EXPECT_CALL(led_, cb_register(_, channel, &led::fade_cb_null, nullptr))
            .WillOnce(Return(ESP_OK));
        ch_.emplace(channel, pin);
    }

  protected:
    const ledc_channel_t channel = led::PWM::channel::RED;
    const gpio_num_t pin = led::pin::RED;

    Handle<MockDftEventLoop> eloop_handle{};
    MockDftEventLoop &eloop{eloop_handle.get()};

    MockLED &led_{MockLED::reset_and_get()};
    led::PendingFade *pen_{};
    std::optional<led::Channel<MockLED, Handle<MockDftEventLoop>>> ch_;
};

TEST_F(ChannelTest, getDutySuccess) {
    EXPECT_CALL(led_, get_duty(led::PWM::MODE, channel, _))
        .WillOnce(DoAll(SetArgReferee<2>(100), Return(ESP_OK)));
    ASSERT_EQ(100, ch_->get_duty());
}

TEST_F(ChannelTest, getDutyFail) {
    EXPECT_CALL(led_, get_duty(led::PWM::MODE, channel, _))
        .WillOnce(DoAll(SetArgReferee<2>(100), Return(ESP_FAIL)));
    ASSERT_THROW(ch_->get_duty(), RuntimeError);
}

TEST_F(ChannelTest, fade) {
    auto tgt_duty = 100;
    EXPECT_CALL(led_, set_fade_time_and_start(_, channel, tgt_duty, _, _))
        .WillOnce(Return(ESP_OK));
    ASSERT_EQ(ESP_OK, ch_->fade(tgt_duty));
    auto param = ledc_cb_param_t{.channel = channel};
    (led::fade_cb_s.fade_cb)(&param, pen_);
}

TEST_F(ChannelTest, fade2Times) {
    auto tgt_duty1 = 100;
    {
        EXPECT_CALL(led_, set_fade_time_and_start(_, channel, tgt_duty1, _, _))
            .WillOnce(Return(ESP_OK));
        ASSERT_EQ(ESP_OK, ch_->fade(tgt_duty1));
    }

    auto tgt_duty2 = 200;
    {
        EXPECT_CALL(led_, set_fade_time_and_start(_, _, _, _, _)).Times(0);
        ASSERT_EQ(ESP_OK, ch_->fade(tgt_duty2));
    }

    EXPECT_CALL(led_, set_fade_time_and_start(_, channel, tgt_duty2, _, _))
        .WillOnce(Return(ESP_OK));
    auto param = ledc_cb_param_t{.channel = channel};
    (led::fade_cb_s.fade_cb)(&param, pen_);
    eloop.wait_for_event();
}
