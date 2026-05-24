#ifndef RUNTIME_STATE_HPP
#define RUNTIME_STATE_HPP

#include "runtime_data.hpp"
#include "util.hpp"
#include <variant>

namespace runtime {

using Data = std::variant<ConnectionErrorData, ErrorData>;

class DataPrinter {
  public:
    const char *operator()(const ConnectionErrorData &) const noexcept;
    const char *operator()(const ErrorData &) const noexcept;
};

class IdleStateData {};
class ReconnectStateData {};

class StateDataPrinter {
  public:
    const char *operator()(const std::monostate &) const noexcept;
    const char *operator()(const IdleStateData &) const noexcept;
    const char *operator()(const ReconnectStateData &) const noexcept;
};

using StateData =
    std::variant<std::monostate, IdleStateData, ReconnectStateData>;
using runtime_t = util::Notifiable<runtime::Data>;

class BaseState : public sm::State<StateData, Data> {
  public:
    BaseState(runtime_t &runtime) noexcept;
    StateData get_next_state_data() const noexcept override;

  protected:
    std::reference_wrapper<runtime_t> runtime;
    StateData next_state_data;
};

class IdleState : public BaseState {
  public:
    IdleState(runtime_t &runtime) noexcept;
    int perform(Data &data) override;
};

class ReconnectState : public BaseState {
  public:
    ReconnectState(runtime_t &runtime) noexcept;
    int perform(Data &data) override;
};

} // namespace runtime

#endif
