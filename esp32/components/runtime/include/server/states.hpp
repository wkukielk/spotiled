#ifndef RUNTIME_STATE_HPP
#define RUNTIME_STATE_HPP

#include "runtime_data.hpp"
#include "util.hpp"
#include <variant>

namespace server {
class IdleStateData {};

class StateDataPrinter {
  public:
    const char *operator()(const std::monostate &) const noexcept;
    const char *operator()(const IdleStateData &) const noexcept;
};

using StateData = std::variant<std::monostate, IdleStateData>;

class BaseState : public sm::State<StateData, Data> {
  public:
    BaseState() noexcept;
    StateData get_next_state_data() const noexcept override;

  protected:
    StateData next_state_data;
};

template <typename I_WSServer, typename I_Queue>
class IdleState : public BaseState {
  public:
    IdleState(I_WSServer &ws_server, I_Queue &queue) noexcept;
    int perform() override;

  private:
    std::reference_wrapper<I_WSServer> ws_server;
    std::reference_wrapper<I_Queue> queue;
};

template <typename I_WSServer, typename I_Queue> class StateFactory {
    using State = sm::State<StateData, Data>;

  public:
    StateFactory(I_WSServer &ws_server, I_Queue &queue);
    std::unique_ptr<State> get(StateData &&data) const;

  private:
    std::reference_wrapper<I_WSServer> ws_server;
    std::reference_wrapper<I_Queue> queue;
};
} // namespace server

#include "server/states.tpp"

#endif
