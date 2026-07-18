#ifndef UDP_SERVER_HPP
#include "UDPServer.hpp"
#endif

#include <array>

template <I_UDPSocket Socket>
std::optional<RGB> UDPServer<Socket>::receive(uint32_t wait_ms) {
    std::array<uint8_t, 3> buf{};
    auto len = socket.receive(buf, wait_ms);
    if (!len || *len != buf.size())
        return std::nullopt;
    return RGB{buf[0], buf[1], buf[2]};
}
