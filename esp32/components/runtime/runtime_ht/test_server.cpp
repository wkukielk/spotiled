#include "TestableCommandHandler.hpp"
#include "TestableServerThread.hpp"
#include <gtest/gtest.h>

using namespace testing;

// --- parse ---

TEST(ParseTest, FadeLed) {
    auto result = server::parse(R"({"cmd":"fade","r":255,"g":0,"b":128})");
    ASSERT_TRUE(result.has_value());
    auto *fade = std::get_if<server::FadeLed>(&*result);
    ASSERT_NE(fade, nullptr);
    EXPECT_EQ(fade->target, (RGB{255, 0, 128}));
}

TEST(ParseTest, InvalidJson) {
    EXPECT_FALSE(server::parse("not json").has_value());
}

TEST(ParseTest, UnknownCommand) {
    EXPECT_FALSE(server::parse(R"({"cmd":"unknown"})").has_value());
}

TEST(ParseTest, MissingFields) {
    EXPECT_FALSE(server::parse(R"({"cmd":"fade","r":255})").has_value());
}

// --- CommandHandler ---

class CommandHandlerTest : public Test {
  protected:
    TestableCommandHandler handler{};
    MockLEDC &ledc = handler.get_ledc();
};

TEST_F(CommandHandlerTest, FadeLedOk) {
    EXPECT_CALL(ledc, fade_to(RGB{255, 0, 128})).WillOnce(Return(ESP_OK));
    auto response = handler.process(
        MockWSServer::make_message(R"({"cmd":"fade","r":255,"g":0,"b":128})"));
    EXPECT_EQ(response, server::Response::ok);
}

TEST_F(CommandHandlerTest, FadeLedFail) {
    EXPECT_CALL(ledc, fade_to(_)).WillOnce(Return(ESP_FAIL));
    auto response = handler.process(
        MockWSServer::make_message(R"({"cmd":"fade","r":255,"g":0,"b":128})"));
    EXPECT_EQ(response, server::Response::fail);
}

TEST_F(CommandHandlerTest, InvalidJson) {
    EXPECT_CALL(ledc, fade_to(_)).Times(0);
    auto response = handler.process(MockWSServer::make_message("bad json"));
    EXPECT_EQ(response, server::Response::invalid_command);
}

TEST_F(CommandHandlerTest, NullBuffer) {
    EXPECT_CALL(ledc, fade_to(_)).Times(0);
    auto response = handler.process({nullptr, 0});
    EXPECT_EQ(response, server::Response::invalid_command);
}

// --- ServerThread::perform ---

class ServerThreadTest : public Test {
  protected:
    MockEventGroup egroup{};
    TestableServerThread server{util::JoinEventGroup{egroup, 0x1}};
    MockWSServer &ws_server = server.get_ws_server();
    MockCommandHandler &handler = server.get_handler();
    MockQueue &queue = server.get_queue();

    void SetUp() override {
        ON_CALL(queue, receive(_)).WillByDefault([](uint32_t) {
            return nullptr;
        });
    }
};

TEST_F(ServerThreadTest, PerformForwardsMessageToHandler) {
    auto msg =
        MockWSServer::make_message(R"({"cmd":"fade","r":255,"g":0,"b":0})");
    EXPECT_CALL(ws_server, receive(1000)).WillOnce(Return(std::move(msg)));
    EXPECT_CALL(handler, process(_));
    server.perform();
}

TEST_F(ServerThreadTest, PerformWithEmptyMessage) {
    EXPECT_CALL(ws_server, receive(1000))
        .WillOnce(Return(ByMove(server::Message{nullptr, 0})));
    EXPECT_CALL(handler, process(_));
    server.perform();
}
