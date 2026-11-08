#include "FakeButtonDeps.hpp"
#include "button.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace std::chrono_literals;

class ButtonTest : public testing::Test {
  protected:
    Button<FakeButtonTypes> button{};
    FakeGPIO &gpio = *FakeGPIO::instance;
    FakePeriodicTimer &probe = *FakePeriodicTimer::instance;
    FakeOneShotTimer &press = *FakeOneShotTimer::instance;

    void set_pressed(bool pressed) {
        gpio.level = pressed ? gpio::level::low : gpio::level::high;
        probe.fire();
    }

    // Long-press timeout is 5s, so time held = 5s - remaining().
    // Like the real timer, remaining() is nullopt once stopped.
    void expect_held(std::chrono::microseconds held) {
        EXPECT_CALL(press, remaining())
            .WillOnce([this, held]() -> std::optional<std::chrono::microseconds> {
                if (!press.is_active())
                    return std::nullopt;
                return 5s - held;
            });
    }
};

TEST_F(ButtonTest, ConfiguresGpioWithPullUp) {
    ASSERT_EQ(gpio.pin, GPIO_NUM_2);
    ASSERT_EQ(gpio.pull, gpio::pull::up);
}

TEST_F(ButtonTest, StartsPollingOnConstruction) {
    ASSERT_TRUE(probe.is_active());
}

TEST_F(ButtonTest, NoPressReportsNothing) {
    probe.fire();
    probe.fire();
    ASSERT_EQ(button.get_last_press(), btn_press::no);
}

TEST_F(ButtonTest, HeldButtonReportsNothingBeforeLongTimeout) {
    set_pressed(true);
    set_pressed(true);
    ASSERT_EQ(button.get_last_press(), btn_press::no);
}

TEST_F(ButtonTest, PressArmsLongPressTimer) {
    set_pressed(true);
    ASSERT_TRUE(press.is_active());
}

TEST_F(ButtonTest, ShortPressReportedOnRelease) {
    expect_held(2s);
    set_pressed(true);
    set_pressed(false);
    ASSERT_EQ(button.get_last_press(), btn_press::short_);
}

TEST_F(ButtonTest, PressShorterThanOneSecondIgnored) {
    expect_held(500ms);
    set_pressed(true);
    set_pressed(false);
    ASSERT_EQ(button.get_last_press(), btn_press::no);
}

TEST_F(ButtonTest, PressOfExactlyOneSecondIsShortPress) {
    expect_held(1s);
    set_pressed(true);
    set_pressed(false);
    ASSERT_EQ(button.get_last_press(), btn_press::short_);
}

TEST_F(ButtonTest, ShortPressCancelsLongPressTimer) {
    expect_held(2s);
    set_pressed(true);
    set_pressed(false);
    ASSERT_FALSE(press.is_active());
}

TEST_F(ButtonTest, IgnoredPressCancelsLongPressTimer) {
    expect_held(500ms);
    set_pressed(true);
    set_pressed(false);
    ASSERT_FALSE(press.is_active());
}

TEST_F(ButtonTest, LongPressReportedWhenTimerFires) {
    set_pressed(true);
    press.fire();
    ASSERT_EQ(button.get_last_press(), btn_press::long_);
}

TEST_F(ButtonTest, ReleaseAfterLongPressIsNotShortPress) {
    EXPECT_CALL(press, remaining()).Times(0);
    set_pressed(true);
    press.fire();
    set_pressed(false);
    ASSERT_EQ(button.get_last_press(), btn_press::long_);
    ASSERT_EQ(button.get_last_press(), btn_press::no);
}

TEST_F(ButtonTest, ReleaseAfterConsumedLongPressIsNotShortPress) {
    set_pressed(true);
    press.fire();
    ASSERT_EQ(button.get_last_press(), btn_press::long_);
    set_pressed(false);
    ASSERT_EQ(button.get_last_press(), btn_press::no);
}

TEST_F(ButtonTest, GetLastPressClearsPending) {
    expect_held(2s);
    set_pressed(true);
    set_pressed(false);
    ASSERT_EQ(button.get_last_press(), btn_press::short_);
    ASSERT_EQ(button.get_last_press(), btn_press::no);
}

TEST_F(ButtonTest, UnreadPressIsNotOverwritten) {
    expect_held(2s);
    set_pressed(true);
    press.fire();
    set_pressed(false);
    set_pressed(true);
    set_pressed(false);
    ASSERT_EQ(button.get_last_press(), btn_press::long_);
}

TEST_F(ButtonTest, SecondPressReportedAfterFirstConsumed) {
    expect_held(2s);
    set_pressed(true);
    set_pressed(false);
    ASSERT_EQ(button.get_last_press(), btn_press::short_);
    set_pressed(true);
    press.fire();
    ASSERT_EQ(button.get_last_press(), btn_press::long_);
}