#include "TestableAnimator.hpp"
#include "freertos/task.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace testing;

class AnimatorTest : public testing::Test {
  protected:
    TestableAnimator animator{};
    MockLEDC &ledc = animator.get_ledc();
};

TEST_F(AnimatorTest, FadeToForwardsWhenIdle) {
    EXPECT_CALL(ledc, fade_to(RGB{1, 2, 3}, 500)).WillOnce(Return(ESP_OK));
    ASSERT_EQ(animator.fade_to(RGB{1, 2, 3}, 500), ESP_OK);
}

TEST_F(AnimatorTest, SetColorForwardsWhenIdle) {
    EXPECT_CALL(ledc, set_color(RGB{1, 2, 3})).WillOnce(Return(ESP_OK));
    ASSERT_EQ(animator.set_color(RGB{1, 2, 3}), ESP_OK);
}

TEST_F(AnimatorTest, SetColorAlwaysWinsOverAnimation) {
    RGB last_color{};
    EXPECT_CALL(ledc, fade_to(_, _))
        .WillRepeatedly(DoAll(SaveArg<0>(&last_color), Return(ESP_OK)));
    EXPECT_CALL(ledc, fade_to(_, _))
        .WillOnce(DoAll(SaveArg<0>(&last_color), Return(ESP_OK)));

    animator.start_animation(
        led::AnimationSpec{{RGB{1, 0, 0}, RGB{0, 1, 0}}, 2});
    vTaskDelay(pdMS_TO_TICKS(20));

    RGB override_color{9, 9, 9};
    ASSERT_EQ(animator.fade_to(override_color, 500), ESP_OK);
    ASSERT_EQ(last_color, override_color);
}

TEST_F(AnimatorTest, StartAnimationWritesFrames) {
    EXPECT_CALL(ledc, fade_to(_, _)).WillRepeatedly(Return(ESP_OK));

    animator.start_animation(
        led::AnimationSpec{{RGB{1, 0, 0}, RGB{0, 1, 0}}, 5});
    vTaskDelay(pdMS_TO_TICKS(20)); // let a few frames run
    animator.close();
}

TEST_F(AnimatorTest, FadeToAlwaysWinsOverAnimation) {
    RGB last_color{};
    EXPECT_CALL(ledc, fade_to(_, _))
        .WillRepeatedly(DoAll(SaveArg<0>(&last_color), Return(ESP_OK)));

    animator.start_animation(
        led::AnimationSpec{{RGB{1, 0, 0}, RGB{0, 1, 0}}, 2});
    vTaskDelay(pdMS_TO_TICKS(20));

    RGB override_color{9, 9, 9};
    ASSERT_EQ(animator.fade_to(override_color, 500), ESP_OK);
    ASSERT_EQ(last_color, override_color);
}
