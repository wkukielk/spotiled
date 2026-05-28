#ifndef SERVER_SERVER_HPP
#define SERVER_SERVER_HPP

#include "WSServer.hpp"
#include "queue.hpp"
#include "server/states.hpp"

template <typename HTTPD> class Server {
  public:
    Server();
    void change_state(server::StateData &&next_state_data);
    esp_err_t init() noexcept;
    esp_err_t close() noexcept;
    int perform();

  private:
    std::unique_ptr<server::Data> pop_queue();

    WSServer<HTTPD> ws_server;
    util::PointerQueue<server::Data> queue;
    u_ptr<server::BaseState> state{};
    server::StateFactory state_factory;
};

#include "server.tpp"

#endif
