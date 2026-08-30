#ifndef SERVER_COMMAND_HPP
#define SERVER_COMMAND_HPP

#include "rgb.hpp"
#include <memory>
#include <optional>
#include <string_view>
#include <utility>
#include <variant>

namespace server {

using Message = std::pair<std::unique_ptr<uint8_t[]>, size_t>;

enum class Response { ok, fail, invalid_command };

struct FadeLed {
    RGB target;
};

struct PulsateLed {
    RGB target;
    uint32_t duration_ms;
};

struct Reprovision {};

using Command = std::variant<FadeLed, PulsateLed, Reprovision>;

std::optional<Command> parse(std::string_view text);

} // namespace server

#endif
