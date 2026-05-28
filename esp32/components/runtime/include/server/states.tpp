#ifndef RUNTIME_STATE_HPP
#include "server/states.hpp"
#endif

namespace server {
template <typename I_WSServer, typename I_Queue>
IdleState<I_WSServer, I_Queue>::IdleState(I_WSServer &ws_server,
                                          I_Queue &queue) noexcept
    : ws_server{ws_server}, queue{queue} {}

template <typename I_WSServer, typename I_Queue>
int IdleState<I_WSServer, I_Queue>::perform() {
    auto ret = ESP_OK;
    auto delay_ms = 500;
    auto data = queue.get().receive(delay_ms);
    if (data) {
        std::visit(overloaded{[&](ACK &) {}, [&](NACK &) { ret = -1; }}, *data);
    }
    ESP_LOGI("Server", "%s", "Waiting for data");
    auto [ws_data, len] = ws_server.get().receive(delay_ms);
    if (ws_data) {
        ESP_LOGI("Server", "%s", (const char *)ws_data.get());
    }
    return ret;
}

template <typename I_WSServer, typename I_Queue>
StateFactory<I_WSServer, I_Queue>::StateFactory(I_WSServer &ws_server,
                                                I_Queue &queue)
    : ws_server{ws_server}, queue{queue} {}

template <typename I_WSServer, typename I_Queue>
std::unique_ptr<sm::State<StateData, Data>>
StateFactory<I_WSServer, I_Queue>::get(StateData &&data) const {
    auto state = std::visit(
        overloaded{[&](IdleStateData &&) {
                       return mkuptr<IdleState>(ws_server, queue);
                   },
                   [](std::monostate &&) {
                       return u_ptr<IdleState<I_WSServer, I_Queue>>{nullptr};
                   }},
        std::move(data));
    UTIL_THROW(!state, NullError, "state factory got monostate");
    return state;
}
} // namespace server