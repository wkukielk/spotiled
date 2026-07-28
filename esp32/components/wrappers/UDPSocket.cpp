#include "UDPSocket.hpp"
#include "esp_log.h"
#include <arpa/inet.h>
#include <cerrno>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

UDPSocket::~UDPSocket() noexcept {
    if (fd >= 0) {
        close(fd);
        ESP_LOGW("UDPSocket",
                 "Call UDPSocket::stop() before destructing UDPSocket");
    }
}

esp_err_t UDPSocket::start() noexcept {
    fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd < 0) {
        ESP_LOGE("UDPSocket", "socket() failed: errno %d", errno);
        return ESP_FAIL;
    }
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if (bind(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) != 0) {
        ESP_LOGE("UDPSocket", "bind() failed: errno %d", errno);
        close(fd);
        fd = -1;
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t UDPSocket::stop() noexcept {
    if (fd < 0)
        return ESP_OK;
    auto ret = close(fd) == 0 ? ESP_OK : ESP_FAIL;
    fd = -1;
    return ret;
}

std::optional<size_t> UDPSocket::receive(std::span<uint8_t> buf,
                                         uint32_t wait_ms) noexcept {
    pollfd pfd{.fd = fd, .events = POLLIN, .revents = 0};
    auto ready = poll(&pfd, 1, static_cast<int>(wait_ms));
    if (ready <= 0)
        return std::nullopt;
    auto n = recvfrom(fd, buf.data(), buf.size(), 0, nullptr, nullptr);
    if (n < 0)
        return std::nullopt;
    return static_cast<size_t>(n);
}
