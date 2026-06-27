#ifndef SERVER_SERVER_HPP
#define SERVER_SERVER_HPP

#include "WSServer.hpp"
#include "queue.hpp"

template <typename I_WSServer, typename I_Queue>
class ServerThread : public util::PostponedThread,
                     public util::Notifiable<server::Data> {
  public:
    ServerThread(const util::JoinEventGroup &join_group);
    esp_err_t start() noexcept;
    esp_err_t close() noexcept;
    void notify(u_ptr<server::Data> &&data) override { queue.send(move(data)); }

  private:
    std::unique_ptr<server::Data> pop_queue();
    int perform();

    I_WSServer::Message receive_from_ws_server();
    static void server_thread_fun(void *server_ptr);

    I_WSServer ws_server;
    I_Queue queue;
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
