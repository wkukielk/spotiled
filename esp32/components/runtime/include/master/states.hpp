#ifndef RUNTIME_STATE_HPP
#define RUNTIME_STATE_HPP

#include "WiFiManager.hpp"
#include "runtime_data.hpp"
#include "util.hpp"
#include <variant>

namespace master {

using Data = std::variant<ConnectionErrorData, ErrorData>;

class DataPrinter {
  public:
    const char *operator()(const ConnectionErrorData &) const noexcept;
    const char *operator()(const ErrorData &) const noexcept;
};

class IdleStateData {};

class StateDataPrinter {
  public:
    const char *operator()(const std::monostate &) const noexcept;
    const char *operator()(const IdleStateData &) const noexcept;
};

using StateData = std::variant<std::monostate, IdleStateData>;
using master_t = util::Notifiable<master::Data>;

class BaseState : public sm::State<StateData, Data> {
  public:
    BaseState() noexcept;
    StateData get_next_state_data() const noexcept override;

  protected:
    StateData next_state_data;
};

class IdleState : public BaseState {
  public:
    IdleState(I_WiFiManager &wifi_manager) noexcept;
    int perform(Data &data) override;

  private:
    std::reference_wrapper<I_WiFiManager> wifi_manager;
};

} // namespace master

#endif
