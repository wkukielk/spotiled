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

class IdleState : public BaseState {
  public:
    IdleState() noexcept;
    int perform(Data &data) override;
};

class StateFactory {
    using State = sm::State<StateData, Data>;

  public:
    std::unique_ptr<State> get(StateData &&data) const;
};
} // namespace server

#endif
