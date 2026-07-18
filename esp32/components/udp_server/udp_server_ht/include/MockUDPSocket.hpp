#ifndef MOCK_UDP_SOCKET_HPP
#define MOCK_UDP_SOCKET_HPP

#include "I_UDPSocket.hpp"
#include <gmock/gmock.h>

class MockUDPSocket {
  public:
    explicit MockUDPSocket(uint16_t) {};
    MOCK_METHOD(esp_err_t, start, (), (noexcept));
    MOCK_METHOD(esp_err_t, stop, (), (noexcept));
    MOCK_METHOD(std::optional<size_t>, receive,
                (std::span<uint8_t>, uint32_t), ());
};

static_assert(I_UDPSocket<MockUDPSocket>);

#endif
