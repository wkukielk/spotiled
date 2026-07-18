#ifndef I_NTERFACES_UDP_SOCKET_HPP
#define I_NTERFACES_UDP_SOCKET_HPP

#include "esp_err.h"
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

template <typename T>
concept I_UDPSocket = std::constructible_from<T, uint16_t> &&
                      requires(T sock, std::span<uint8_t> buf, uint32_t wait) {
                          { sock.start() } -> std::same_as<esp_err_t>;
                          { sock.stop() } -> std::same_as<esp_err_t>;
                          {
                              sock.receive(buf, wait)
                          } -> std::same_as<std::optional<size_t>>;
                      };

#endif
