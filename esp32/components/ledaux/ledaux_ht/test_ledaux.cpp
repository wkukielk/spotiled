#include "TestableLEDC.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace testing;

class LedauxTest : public testing::Test {
  protected:
    TestableLEDC ledc{};
    MockLED &mock_led{ledc.get_led()};
};

TEST_F(LedauxTest, setColor) {
    RGB color{100, 150, 200};

    InSequence seq;
    EXPECT_CALL(mock_led,
                set_fade_time_and_start(led::PWM::MODE, led::PWM::channel::RED, 100,
                                        led::PWM::DEFAULT_FADE_TIME_MS, LEDC_FADE_NO_WAIT))
        .WillOnce(Return(ESP_OK));
    EXPECT_CALL(mock_led,
                set_fade_time_and_start(led::PWM::MODE, led::PWM::channel::GREEN, 150,
                                        led::PWM::DEFAULT_FADE_TIME_MS, LEDC_FADE_NO_WAIT))
        .WillOnce(Return(ESP_OK));
    EXPECT_CALL(mock_led,
                set_fade_time_and_start(led::PWM::MODE, led::PWM::channel::BLUE, 200,
                                        led::PWM::DEFAULT_FADE_TIME_MS, LEDC_FADE_NO_WAIT))
        .WillOnce(Return(ESP_OK));

    ASSERT_EQ(ledc.set_color(color), ESP_OK);
}

TEST_F(LedauxTest, setColorStopsOnFirstError) {
    RGB color{100, 150, 200};

    EXPECT_CALL(mock_led,
                set_fade_time_and_start(led::PWM::MODE, led::PWM::channel::RED, _, _, _))
        .WillOnce(Return(ESP_FAIL));
    EXPECT_CALL(mock_led, set_fade_time_and_start(led::PWM::MODE, led::PWM::channel::GREEN,
                                                   _, _, _))
        .Times(0);
    EXPECT_CALL(mock_led, set_fade_time_and_start(led::PWM::MODE, led::PWM::channel::BLUE,
                                                   _, _, _))
        .Times(0);

    ASSERT_EQ(ledc.set_color(color), ESP_FAIL);
}

TEST_F(LedauxTest, getCurrentColor) {
    EXPECT_CALL(mock_led, get_duty(led::PWM::MODE, led::PWM::channel::RED))
        .WillOnce(Return(100));
    EXPECT_CALL(mock_led, get_duty(led::PWM::MODE, led::PWM::channel::GREEN))
        .WillOnce(Return(150));
    EXPECT_CALL(mock_led, get_duty(led::PWM::MODE, led::PWM::channel::BLUE))
        .WillOnce(Return(200));

    auto color = ledc.get_current_color();
    ASSERT_EQ(color.r, 100);
    ASSERT_EQ(color.g, 150);
    ASSERT_EQ(color.b, 200);
}
