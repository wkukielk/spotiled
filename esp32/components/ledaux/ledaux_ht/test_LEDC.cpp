#include "MockDftEventLoop.hpp"
#include "MockLED.hpp"
#include "ledaux.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace testing;

using namespace led::PWM;

class LEDCTest : public testing::Test {
  protected:
    DftEventLoopConfigurer dft_eloop{};
    MockLED &led_{MockLED::reset_and_get()};
    LEDC<MockLED, MockDftEventLoop> ledc{};
};

TEST_F(LEDCTest, setColor) {
    RGB color{100, 150, 200};

    InSequence seq;
    EXPECT_CALL(led_, set_fade_time_and_start(MODE, channel::RED, 100, _, _))
        .WillOnce(Return(ESP_OK));
    EXPECT_CALL(led_, set_fade_time_and_start(MODE, channel::GREEN, 150, _, _))
        .WillOnce(Return(ESP_OK));
    EXPECT_CALL(led_, set_fade_time_and_start(MODE, channel::BLUE, 200, _, _))
        .WillOnce(Return(ESP_OK));

    ASSERT_EQ(ledc.set_color(color), ESP_OK);
}

TEST_F(LEDCTest, setColorStopsOnFirstError) {
    RGB color{100, 150, 200};

    EXPECT_CALL(led_, set_fade_time_and_start(MODE, channel::RED, _, _, _))
        .WillOnce(Return(ESP_FAIL));
    EXPECT_CALL(led_, set_fade_time_and_start(MODE, channel::GREEN, _, _, _))
        .Times(0);
    EXPECT_CALL(led_, set_fade_time_and_start(MODE, channel::BLUE, _, _, _))
        .Times(0);

    ASSERT_EQ(ledc.set_color(color), ESP_FAIL);
}

TEST_F(LEDCTest, getCurrentColor) {
    auto MyMock = [](uint32_t value) {
        return DoAll(SetArgReferee<2>(value), Return(ESP_OK));
    };

    EXPECT_CALL(led_, get_duty(MODE, channel::RED, _)).WillOnce(MyMock(100));
    EXPECT_CALL(led_, get_duty(MODE, channel::GREEN, _)).WillOnce(MyMock(150));
    EXPECT_CALL(led_, get_duty(MODE, channel::BLUE, _)).WillOnce(MyMock(200));

    auto color = ledc.get_current_color();
    ASSERT_EQ(color.r, 100);
    ASSERT_EQ(color.g, 150);
    ASSERT_EQ(color.b, 200);
}
