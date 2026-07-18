#ifndef WRAPPERS_UDP_SOCKET_HPP
#define WRAPPERS_UDP_SOCKET_HPP

#include "I_UDPSocket.hpp"
#include "esp_err.h"
#include <cstdint>
#include <optional>
#include <span>

class UDPSocket {
  public:
    explicit UDPSocket(uint16_t port) : port{port} {};
    ~UDPSocket() noexcept;
    esp_err_t start() noexcept;
    esp_err_t stop() noexcept;
    std::optional<size_t> receive(std::span<uint8_t> buf,
                                  uint32_t wait_ms) noexcept;

  private:
    uint16_t port;
    int fd = -1;
};

static_assert(I_UDPSocket<UDPSocket>);

#endif
