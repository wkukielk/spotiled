#ifndef UDP_SERVER_HPP
#define UDP_SERVER_HPP

#include "I_UDPSocket.hpp"
#include "rgb.hpp"
#include <cstdint>
#include <optional>

namespace udp_server {
inline constexpr uint16_t port = 8001;
}

template <I_UDPSocket Socket> class UDPServer {
  public:
    UDPServer() : socket{udp_server::port} {};
    esp_err_t start() noexcept { return socket.start(); }
    esp_err_t stop() noexcept { return socket.stop(); }
    std::optional<RGB> receive(uint32_t wait_ms);

  protected:
    Socket socket;
};

#include "UDPServer.tpp"

#endif
