#ifndef RUNTIME_HELPERS_HPP
#include "runtime_helpers.hpp"
#endif

template <typename HTTPD> static void server_thread_fun(void *server_ptr) {
    auto server = (Server<HTTPD> *)server_ptr;
    int ret{};
    try {
        while ((ret = server->perform()) == ESP_OK)
            ;
    } catch (const std::exception &e) {
        ESP_LOGE(server_tag, "Exception: %s", e.what());
    }
    ESP_LOGE(server_tag, "Return: %d", ret);
}

template <typename HTTPD>
ServerThread<HTTPD>::ServerThread(Server<HTTPD> &&server,
                                  const JoinEventGroup &join_group)
    : PostponedThread{join_group}, server{move(server)} {}

template <typename HTTPD> void ServerThread<HTTPD>::start() {
    thread.emplace(util::thread_params{.fun = server_thread_fun,
                                       .name = "server",
                                       .fun_parameters = &server,
                                       .join_group = join_group.egroup,
                                       .join_bit = join_group.join_bit});
}
