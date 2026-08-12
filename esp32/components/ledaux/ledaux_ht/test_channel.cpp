#include "MockLED.hpp"
#include "ledaux.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <optional>

using namespace testing;

class ChannelConstructTest : public testing::Test {
  protected:
    const ledc_channel_t channel = led::PWM::channel::RED;
    const gpio_num_t pin = led::pin::RED;

    MockLED &led_{MockLED::reset_and_get()};
};

TEST_F(ChannelConstructTest, configFail) {
    EXPECT_CALL(led_, configure_channel(_)).WillOnce(Return(ESP_FAIL));
    ASSERT_THROW((led::Channel<MockLED>{channel, pin}), InitError);
}

class ChannelTest : public testing::Test {
  public:
    void SetUp() override { ch_.emplace(channel, pin); }

  protected:
    const ledc_channel_t channel = led::PWM::channel::RED;
    const gpio_num_t pin = led::pin::RED;

    MockLED &led_{MockLED::reset_and_get()};
    std::optional<led::Channel<MockLED>> ch_;
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
    InSequence seq;
    EXPECT_CALL(led_, stop_fade(_, channel)).WillOnce(Return(ESP_OK));
    EXPECT_CALL(led_, set_fade_time_and_start(_, channel, 100, 1000, _))
        .WillOnce(Return(ESP_OK));

    ASSERT_EQ(ESP_OK, ch_->fade(100, 1000));
}

TEST_F(ChannelTest, fadeStopsFadeFirst) {
    EXPECT_CALL(led_, stop_fade(_, channel)).WillOnce(Return(ESP_FAIL));
    EXPECT_CALL(led_, set_fade_time_and_start(_, _, _, _, _)).Times(0);

    ASSERT_EQ(ESP_FAIL, ch_->fade(100, 1000));
}

TEST_F(ChannelTest, fadeTwiceStopsPreviousFade) {
    {
        InSequence seq;
        EXPECT_CALL(led_, stop_fade(_, channel)).WillOnce(Return(ESP_OK));
        EXPECT_CALL(led_, set_fade_time_and_start(_, channel, 100, 1000, _))
            .WillOnce(Return(ESP_OK));
        ASSERT_EQ(ESP_OK, ch_->fade(100, 1000));
    }
    {
        InSequence seq;
        EXPECT_CALL(led_, stop_fade(_, channel)).WillOnce(Return(ESP_OK));
        EXPECT_CALL(led_, set_fade_time_and_start(_, channel, 200, 500, _))
            .WillOnce(Return(ESP_OK));
        ASSERT_EQ(ESP_OK, ch_->fade(200, 500));
    }
}

TEST_F(ChannelTest, setInstant) {
    InSequence seq;
    EXPECT_CALL(led_, stop_fade(_, channel)).WillOnce(Return(ESP_OK));
    EXPECT_CALL(led_, set_duty(_, channel, 42)).WillOnce(Return(ESP_OK));
    EXPECT_CALL(led_, update_duty(_, channel)).WillOnce(Return(ESP_OK));

    ASSERT_EQ(ESP_OK, ch_->set_color(42));
}

TEST_F(ChannelTest, setInstantStopsFadeFirst) {
    EXPECT_CALL(led_, stop_fade(_, channel)).WillOnce(Return(ESP_FAIL));
    EXPECT_CALL(led_, set_duty(_, _, _)).Times(0);
    EXPECT_CALL(led_, update_duty(_, _)).Times(0);

    ASSERT_EQ(ESP_FAIL, ch_->set_color(42));
}
