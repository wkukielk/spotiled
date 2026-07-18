#ifndef MOCK_UDP_SERVER_HPP
#define MOCK_UDP_SERVER_HPP

#include "rgb.hpp"
#include <gmock/gmock.h>
#include <optional>

class MockUDPServer {
  public:
    MOCK_METHOD(esp_err_t, start, (), (noexcept));
    MOCK_METHOD(esp_err_t, stop, (), (noexcept));
    MOCK_METHOD(std::optional<RGB>, receive, (uint32_t), ());
};

#endif
