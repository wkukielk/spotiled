#ifndef SERVER_SERVER_HPP
#define SERVER_SERVER_HPP

// #include "WSServer.hpp"
#include "queue.hpp"
#include "runtime_data.hpp"
#include "server/command_handler.hpp"

template <typename T>
concept I_WSServer = requires(T t) {
    { t.receive(int{}) } -> std::same_as<server::Message>;
};

template <typename T>
concept ServerThreadTypes = requires {
    requires I_WSServer<typename T::WSServer>;
    typename T::Queue;
    requires I_CommandHandler<typename T::Handler>;
};

template <ServerThreadTypes T>
class ServerThread : public util::PostponedThread,
                     public util::Notifiable<server::Data> {
    using I_WSServer = typename T::WSServer;
    using I_Queue = typename T::Queue;

  public:
    using Handler = typename T::Handler;

    ServerThread(std::unique_ptr<Handler> &&handler,
                const util::JoinEventGroup &join_group);
    esp_err_t start() noexcept;
    esp_err_t close() noexcept;
    void notify(u_ptr<server::Data> &&data) override { queue.send(move(data)); }

  protected:
    int perform();

    I_WSServer ws_server;
    I_Queue queue;
    std::unique_ptr<Handler> handler;

  private:
    std::unique_ptr<server::Data> pop_queue();
    auto receive_from_ws_server() -> I_WSServer::Message;
    static void server_thread_fun(void *server_ptr);
};

namespace server {
class DataPrinter {
  public:
    const char *operator()(const ACK &) const noexcept { return "ack"; }
    const char *operator()(const NACK &) const noexcept { return "nack"; }
};
} // namespace server

#include "server.tpp"

#endif
