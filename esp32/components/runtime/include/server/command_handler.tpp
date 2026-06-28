#ifndef SERVER_COMMAND_HANDLER_HPP
#include "server/command_handler.hpp"
#endif

template <I_LEDC LEDC>
server::Response CommandHandler<LEDC>::process(server::Message msg) {
    auto &[buf, len] = msg;
    if (!buf)
        return server::Response::invalid_command;
    auto cmd = server::parse({reinterpret_cast<const char *>(buf.get()), len});
    if (!cmd)
        return server::Response::invalid_command;
    return std::visit([this](auto &&c) { return handle(c); }, *cmd);
}

template <I_LEDC LEDC>
server::Response CommandHandler<LEDC>::handle(const server::FadeLed &cmd) {
    return ledc.set_color(cmd.target) == ESP_OK ? server::Response::ok
                                                : server::Response::fail;
}
