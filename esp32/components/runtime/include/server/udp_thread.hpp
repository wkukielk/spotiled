#ifndef SERVER_UDP_THREAD_HPP
#define SERVER_UDP_THREAD_HPP

#include "rgb.hpp"
#include "server/led_color_handler.hpp"
#include "util.hpp"
#include <memory>
#include <optional>

template <typename T>
concept I_UDPServer = requires(T t) {
    { t.receive(uint32_t{}) } -> std::same_as<std::optional<RGB>>;
};

template <typename T>
concept UDPThreadTypes = requires {
    requires I_UDPServer<typename T::UDPServer>;
    requires I_LedColorHandler<typename T::Handler>;
};

template <UDPThreadTypes T> class UDPThread : public util::PostponedThread {
    using I_UDPServer = typename T::UDPServer;

  public:
    using Handler = typename T::Handler;

    UDPThread(std::unique_ptr<Handler> &&handler,
             const util::JoinEventGroup &join_group);
    esp_err_t start() noexcept;
    esp_err_t close() noexcept;

  protected:
    int perform();

    I_UDPServer udp_server;
    std::unique_ptr<Handler> handler;

  private:
    static void udp_thread_fun(void *thread_ptr);
};

#include "udp_thread.tpp"

#endif
