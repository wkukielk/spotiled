#ifndef SERVER_COMMAND_HANDLER_HPP
#include "server/command_handler.hpp"
#endif

#include <algorithm>

struct CommandLogger {
    void operator()(const server::FadeLed &cmd) const {
        ESP_LOGI("CommandHandler", "FadeLed target=(%d,%d,%d)",
                 (int)cmd.target.r, (int)cmd.target.g, (int)cmd.target.b);
    }

    void operator()(const server::PulsateLed &cmd) const {
        ESP_LOGI("CommandHandler",
                 "PulsateLed target=(%d,%d,%d) duration_ms=%d",
                 (int)cmd.target.r, (int)cmd.target.g, (int)cmd.target.b,
                 (int)cmd.duration_ms);
    }
};

template <I_Animatable LEDC>
server::Response CommandHandler<LEDC>::process(server::Message msg) {
    auto &[buf, len] = msg;
    if (!buf)
        return server::Response::invalid_command;
    auto cmd = server::parse({reinterpret_cast<const char *>(buf.get()), len});
    if (!cmd)
        return server::Response::invalid_command;
    auto visitor = [this](auto &&c) {
        CommandLogger{}(c);
        return this->handle(c);
    };
    return std::visit(visitor, *cmd);
}

template <I_Animatable LEDC>
server::Response CommandHandler<LEDC>::handle(const server::FadeLed &cmd) {
    return ledc.fade_to(cmd.target, led::PWM::DEFAULT_FADE_TIME_MS) == ESP_OK
               ? server::Response::ok
               : server::Response::fail;
}

template <I_Animatable LEDC>
server::Response CommandHandler<LEDC>::handle(const server::PulsateLed &cmd) {
    uint32_t half_period_ms = std::max<uint32_t>(1, cmd.duration_ms / 2);
    ledc.start_animation(
        led::AnimationSpec{{cmd.target, RGB{0, 0, 0}}, half_period_ms});
    return server::Response::ok;
}
