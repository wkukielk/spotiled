#ifndef SERVER_SERVER_HPP
#include "server/server.hpp"
#endif

template <typename I_WSServer, typename I_Queue>
void ServerThread<I_WSServer, I_Queue>::server_thread_fun(void *server_ptr) {
    auto server = (ServerThread<I_WSServer, I_Queue> *)server_ptr;
    int ret{};
    try {
        while ((ret = server->perform()) == ESP_OK)
            ;
    } catch (const std::exception &e) {
        ESP_LOGE("Server", "Exception: %s", e.what());
    }
    ESP_LOGE("Server", "Return: %d", ret);
}

template <typename I_WSServer, typename I_Queue>
ServerThread<I_WSServer, I_Queue>::ServerThread(
    const util::JoinEventGroup &join_group)
    : util::PostponedThread{join_group}, ws_server{}, queue{4} {}

template <typename I_WSServer, typename I_Queue>
esp_err_t ServerThread<I_WSServer, I_Queue>::start() noexcept {
    thread.emplace(util::thread_params{.fun = server_thread_fun,
                                       .name = "server",
                                       .fun_parameters = this,
                                       .join_group = join_group.egroup,
                                       .join_bit = join_group.join_bit});
    return ws_server.start();
}

template <typename I_WSServer, typename I_Queue>
esp_err_t ServerThread<I_WSServer, I_Queue>::close() noexcept {
    return ws_server.stop();
}

template <typename I_WSServer, typename I_Queue>
int ServerThread<I_WSServer, I_Queue>::perform() {
    auto ret = ESP_OK;
    auto data = pop_queue();

    auto [ws_data, len] = receive_from_ws_server();
    return ret;
}

template <typename I_WSServer, typename I_Queue>
I_WSServer::Message
ServerThread<I_WSServer, I_Queue>::receive_from_ws_server() {
    const auto delay_ms = 1000;
    ESP_LOGI("Server", "%s", "Waiting for data");
    auto message = ws_server.receive(delay_ms);
    auto &ws_data = message.first;
    if (ws_data) {
        ESP_LOGI("Server", "%s", (const char *)ws_data.get());
    }
    return message;
}

template <typename I_WSServer, typename I_Queue>
std::unique_ptr<server::Data> ServerThread<I_WSServer, I_Queue>::pop_queue() {
    const auto delay_ms = 1000;
    auto data = queue.receive(delay_ms);
    if (data.get())
        ESP_LOGI("Server", "Queue: %s\n",
                 std::visit(server::DataPrinter{}, *data));
    return data;
}
