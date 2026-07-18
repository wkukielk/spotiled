#ifndef SERVER_SERVER_HPP
#include "server/server.hpp"
#endif

template <ServerThreadTypes T>
void ServerThread<T>::server_thread_fun(void *server_ptr) {
    auto server = (ServerThread<T> *)server_ptr;
    int ret{};
    try {
        while ((ret = server->perform()) == ESP_OK)
            ;
    } catch (const std::exception &e) {
        ESP_LOGE("Server", "Exception: %s", e.what());
    }
    ESP_LOGE("Server", "Return: %d", ret);
}

template <ServerThreadTypes T>
ServerThread<T>::ServerThread(const util::JoinEventGroup &join_group)
    : util::PostponedThread{join_group}, ws_server{}, queue{4}, handler{} {}

template <ServerThreadTypes T> esp_err_t ServerThread<T>::start() noexcept {
    thread.emplace(util::thread_params{.fun = server_thread_fun,
                                       .name = "server",
                                       .fun_parameters = this,
                                       .join_group = join_group.egroup,
                                       .join_bit = join_group.join_bit});
    return ws_server.start();
}

template <ServerThreadTypes T> esp_err_t ServerThread<T>::close() noexcept {
    return ws_server.stop();
}

template <ServerThreadTypes T> int ServerThread<T>::perform() {
    auto ret = ESP_OK;
    auto data = pop_queue();
    auto res = handler.process(receive_from_ws_server());
    ESP_LOGI("ServerThread", "response: %d", (int)res);
    return ret;
}

template <ServerThreadTypes T>
auto ServerThread<T>::receive_from_ws_server() ->
    typename T::WSServer::Message {
    const auto delay_ms = 1000;
    auto message = ws_server.receive(delay_ms);
    if (message.first)
        ESP_LOGI("Server", "%s", (const char *)message.first.get());
    return message;
}

template <ServerThreadTypes T>
std::unique_ptr<server::Data> ServerThread<T>::pop_queue() {
    const auto delay_ms = 1000;
    auto data = queue.receive(delay_ms);
    if (data.get())
        ESP_LOGI("Server", "Queue: %s\n",
                 std::visit(server::DataPrinter{}, *data));
    return data;
}
