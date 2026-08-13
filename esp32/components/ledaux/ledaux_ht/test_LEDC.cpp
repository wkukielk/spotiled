#include "Handle.hpp"
#include "MockLED.hpp"
#include "ledaux.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace testing;

using namespace led::PWM;

class LEDCTest : public testing::Test {
  protected:
    Handle<NiceMock<MockLED>> led_handle{};
    NiceMock<MockLED> &led_{led_handle.get()};
    LEDC<NiceMock<MockLED>> ledc{};
};

TEST_F(LEDCTest, fadeTo) {
    RGB color{100, 150, 200};

    EXPECT_CALL(led_, stop_fade(_, _)).WillRepeatedly(Return(ESP_OK));
    InSequence seq;
    EXPECT_CALL(led_, set_fade_time_and_start(MODE, channel::RED, 100, 2000, _))
        .WillOnce(Return(ESP_OK));
    EXPECT_CALL(led_,
                set_fade_time_and_start(MODE, channel::GREEN, 150, 2000, _))
        .WillOnce(Return(ESP_OK));
    EXPECT_CALL(led_,
                set_fade_time_and_start(MODE, channel::BLUE, 200, 2000, _))
        .WillOnce(Return(ESP_OK));

    ASSERT_EQ(ledc.fade_to(color, 2000), ESP_OK);
}

TEST_F(LEDCTest, fadeToStopsOnFirstError) {
    RGB color{100, 150, 200};

    EXPECT_CALL(led_, stop_fade(_, _)).WillRepeatedly(Return(ESP_OK));
    EXPECT_CALL(led_, set_fade_time_and_start(MODE, channel::RED, _, _, _))
        .WillOnce(Return(ESP_FAIL));
    EXPECT_CALL(led_, set_fade_time_and_start(MODE, channel::GREEN, _, _, _))
        .Times(0);
    EXPECT_CALL(led_, set_fade_time_and_start(MODE, channel::BLUE, _, _, _))
        .Times(0);

    ASSERT_EQ(ledc.fade_to(color, 2000), ESP_FAIL);
}

TEST_F(LEDCTest, setColor) {
    RGB color{100, 150, 200};

    InSequence seq;
    EXPECT_CALL(led_, stop_fade(MODE, channel::RED)).WillOnce(Return(ESP_OK));
    EXPECT_CALL(led_, set_duty(MODE, channel::RED, 100))
        .WillOnce(Return(ESP_OK));
    EXPECT_CALL(led_, update_duty(MODE, channel::RED)).WillOnce(Return(ESP_OK));
    EXPECT_CALL(led_, stop_fade(MODE, channel::GREEN)).WillOnce(Return(ESP_OK));
    EXPECT_CALL(led_, set_duty(MODE, channel::GREEN, 150))
        .WillOnce(Return(ESP_OK));
    EXPECT_CALL(led_, update_duty(MODE, channel::GREEN))
        .WillOnce(Return(ESP_OK));
    EXPECT_CALL(led_, stop_fade(MODE, channel::BLUE)).WillOnce(Return(ESP_OK));
    EXPECT_CALL(led_, set_duty(MODE, channel::BLUE, 200))
        .WillOnce(Return(ESP_OK));
    EXPECT_CALL(led_, update_duty(MODE, channel::BLUE))
        .WillOnce(Return(ESP_OK));

    ASSERT_EQ(ledc.set_color(color), ESP_OK);
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
