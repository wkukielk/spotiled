#include "TestableUDPServer.hpp"
#include <gtest/gtest.h>

using namespace testing;

class UDPServerTest : public Test {
  protected:
    TestableUDPServer server{};
    MockUDPSocket &socket = server.get_socket();
};

TEST_F(UDPServerTest, start) {
    EXPECT_CALL(socket, start()).WillOnce(Return(ESP_OK));
    ASSERT_EQ(server.start(), ESP_OK);
}

TEST_F(UDPServerTest, stop) {
    EXPECT_CALL(socket, stop()).WillOnce(Return(ESP_OK));
    ASSERT_EQ(server.stop(), ESP_OK);
}

TEST_F(UDPServerTest, receiveValidPacket) {
    EXPECT_CALL(socket, receive(_, 1000))
        .WillOnce([](std::span<uint8_t> buf, uint32_t) {
            buf[0] = 255;
            buf[1] = 0;
            buf[2] = 128;
            return std::optional<size_t>{3};
        });
    auto color = server.receive(1000);
    ASSERT_TRUE(color.has_value());
    EXPECT_EQ(*color, (RGB{255, 0, 128}));
}

TEST_F(UDPServerTest, receiveTooShort) {
    EXPECT_CALL(socket, receive(_, 1000)).WillOnce(Return(std::optional<size_t>{2}));
    auto color = server.receive(1000);
    EXPECT_FALSE(color.has_value());
}

TEST_F(UDPServerTest, receiveTimeout) {
    EXPECT_CALL(socket, receive(_, 1000)).WillOnce(Return(std::nullopt));
    auto color = server.receive(1000);
    EXPECT_FALSE(color.has_value());
}
