#include "server/server.hpp"

const char *TAG = "Server";

using namespace std;
using namespace server;

class DataPrinter {
  public:
    const char *operator()(const Command &) const noexcept { return "command"; }
    const char *operator()(const Stop &) const noexcept { return "stop"; }
    const char *operator()(const ACK &) const noexcept { return "ack"; }
    const char *operator()(const NACK &) const noexcept { return "nack"; }
};

Server::Server() : queue{4} {}

void Server::change_state(server::StateData &&next_state_data) {
    UTIL_LOGI("New state: %s\n",
              std::visit(StateDataPrinter{}, next_state_data));
}

int Server::loop() {
    auto data = pop_queue();
    int ret = data ? state->perform(*data) : ESP_OK;
    auto next_state_data = state->get_next_state_data();
    if (!holds_alternative<monostate>(next_state_data))
        change_state(move(next_state_data));
    return ret;
}

std::unique_ptr<server::Data> Server::pop_queue() {
    const auto delay_ms = 5000;
    auto data = queue.receive(delay_ms);
    if (data.get())
        UTIL_LOGI("Queue: %s\n", std::visit(DataPrinter{}, *data));
    return data;
}