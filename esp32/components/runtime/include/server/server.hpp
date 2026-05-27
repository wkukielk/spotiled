#ifndef SERVER_SERVER_HPP
#define SERVER_SERVER_HPP

#include "queue.hpp"
#include "server/states.hpp"

class Server {
  public:
    Server();
    void change_state(server::StateData &&next_state_data);
    int loop();

  private:
    std::unique_ptr<server::Data> pop_queue();

    util::PointerQueue<server::Data> queue;
    u_ptr<server::BaseState> state{};
    server::StateFactory state_factory;
};

#endif
