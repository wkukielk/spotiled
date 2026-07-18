#include "TestableLedColorHandler.hpp"
#include "TestableUDPThread.hpp"
#include <gtest/gtest.h>

using namespace testing;

// --- LedColorHandler ---

class LedColorHandlerTest : public Test {
  protected:
    TestableLedColorHandler handler{};
    MockLEDC &ledc = handler.get_ledc();
};

TEST_F(LedColorHandlerTest, SetsColor) {
    EXPECT_CALL(ledc, set_color(RGB{255, 0, 128})).WillOnce(Return(ESP_OK));
    auto response = handler.process(RGB{255, 0, 128});
    EXPECT_EQ(response, ESP_OK);
}

TEST_F(LedColorHandlerTest, SetColorFails) {
    EXPECT_CALL(ledc, set_color(_)).WillOnce(Return(ESP_FAIL));
    auto response = handler.process(RGB{255, 0, 128});
    EXPECT_EQ(response, ESP_FAIL);
}

TEST_F(LedColorHandlerTest, InvalidPacket) {
    EXPECT_CALL(ledc, set_color(_)).Times(0);
    auto response = handler.process(std::nullopt);
    EXPECT_EQ(response, ESP_ERR_INVALID_ARG);
}

// --- UDPThread::perform ---

class UDPThreadTest : public Test {
  protected:
    MockEventGroup egroup{};
    TestableUDPThread thread{util::JoinEventGroup{egroup, 0x1}};
    MockUDPServer &udp_server = thread.get_udp_server();
    MockLedColorHandler &handler = thread.get_handler();
};

TEST_F(UDPThreadTest, PerformForwardsColorToHandler) {
    EXPECT_CALL(udp_server, receive(1000))
        .WillOnce(Return(std::optional<RGB>{RGB{255, 0, 0}}));
    EXPECT_CALL(handler, process(_));
    thread.perform();
}

TEST_F(UDPThreadTest, PerformWithNoPacket) {
    EXPECT_CALL(udp_server, receive(1000)).WillOnce(Return(std::nullopt));
    EXPECT_CALL(handler, process(_));
    thread.perform();
}
