#ifndef MOCK_WS_SERVER_HPP
#define MOCK_WS_SERVER_HPP

#include "server/command.hpp"
#include <cstring>
#include <gmock/gmock.h>
#include <memory>

class MockWSServer {
  public:
    using Message = server::Message;

    MOCK_METHOD(esp_err_t, start, (), (noexcept));
    MOCK_METHOD(esp_err_t, stop, (), (noexcept));
    MOCK_METHOD(Message, receive, (uint32_t), ());
    MOCK_METHOD(Message, receive_nowait, (), ());

    static Message make_message(std::string_view text) {
        auto buf = std::make_unique<uint8_t[]>(text.size() + 1);
        std::memcpy(buf.get(), text.data(), text.size() + 1);
        return {std::move(buf), text.size()};
    }
};

#endif
