#ifndef SERVER_SERVER_HPP
#include "server/server.hpp"
#endif

template <typename HTTPD>
Server<HTTPD>::Server()
    : ws_server{}, queue{4}, state{mkuptr<server::IdleState>()},
      state_factory{} {}

template <typename HTTPD>
void Server<HTTPD>::change_state(server::StateData &&next_state_data) {
    ESP_LOGI("Server", "New state: %s\n",
             std::visit(server::StateDataPrinter{}, next_state_data));
}

template <typename HTTPD> esp_err_t Server<HTTPD>::init() noexcept {
    return ws_server.start();
}

template <typename HTTPD> esp_err_t Server<HTTPD>::close() noexcept {
    return ws_server.stop();
}

template <typename HTTPD> int Server<HTTPD>::perform() {
    auto data = pop_queue();
    int ret = data ? state->perform(*data) : ESP_OK;
    auto next_state_data = state->get_next_state_data();
    if (!holds_alternative<monostate>(next_state_data))
        change_state(move(next_state_data));
    return ret;
}

template <typename HTTPD>
std::unique_ptr<server::Data> Server<HTTPD>::pop_queue() {
    const auto delay_ms = 5000;
    auto data = queue.receive(delay_ms);
    if (data.get())
        ESP_LOGI("Server", "Queue: %s\n",
                 std::visit(server::DataPrinter{}, *data));
    return data;
}
