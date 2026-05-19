#ifndef RUNTIME_STATE_HPP
#define RUNTIME_STATE_HPP

#include "state_machine.hpp"
#include <variant>

namespace runtime {
enum class DataType { CONNECTION_ERROR, ERROR };

class ConnectionErrorData : public sm::Data<DataType> {
    ConnectionErrorData() : sm::Data<DataType>{DataType::CONNECTION_ERROR} {}
};
class ErrorData : public sm::Data<DataType> {
    ErrorData() : sm::Data<DataType>{DataType::ERROR} {}
};

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

class IdleState : public sm::State<StateData, Data> {
  public:
    IdleState() noexcept;
    int perform(Data &data) override;
    StateData get_next_state_data() const noexcept override;

  private:
    StateData next_state_data;
};

} // namespace runtime

#endif
