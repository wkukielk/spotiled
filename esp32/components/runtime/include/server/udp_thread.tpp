#ifndef SERVER_UDP_THREAD_HPP
#include "server/udp_thread.hpp"
#endif

template <UDPThreadTypes T>
void UDPThread<T>::udp_thread_fun(void *thread_ptr) {
    auto thread = (UDPThread<T> *)thread_ptr;
    int ret{};
    try {
        while ((ret = thread->perform()) == ESP_OK)
            ;
    } catch (const std::exception &e) {
        ESP_LOGE("UDPThread", "Exception: %s", e.what());
    }
    ESP_LOGE("UDPThread", "Return: %d", ret);
}

template <UDPThreadTypes T>
UDPThread<T>::UDPThread(std::unique_ptr<Handler> &&handler,
                        const util::JoinEventGroup &join_group)
    : util::PostponedThread{join_group}, udp_server{},
      handler{std::move(handler)} {}

template <UDPThreadTypes T> esp_err_t UDPThread<T>::start() noexcept {
    thread.emplace(util::thread_params{.fun = udp_thread_fun,
                                       .name = "udp",
                                       .fun_parameters = this,
                                       .join_group = join_group.egroup,
                                       .join_bit = join_group.join_bit});
    return udp_server.start();
}

template <UDPThreadTypes T> esp_err_t UDPThread<T>::close() noexcept {
    return udp_server.stop();
}

template <UDPThreadTypes T> int UDPThread<T>::perform() {
    const auto delay_ms = 1000;
    auto color = udp_server.receive(delay_ms);
    auto res = handler->process(color);
    ESP_LOGI("UDPThread", "response: %d", (int)res);
    return ESP_OK;
}
