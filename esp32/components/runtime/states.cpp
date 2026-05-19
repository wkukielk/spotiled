#include "states.hpp"

namespace runtime {
const char *
DataPrinter::operator()(const ConnectionErrorData &) const noexcept {
    return "connection error";
}

const char *DataPrinter::operator()(const ErrorData &) const noexcept {
    return "error";
}

const char *
StateDataPrinter::operator()(const std::monostate &) const noexcept {
    return "no state";
}

const char *StateDataPrinter::operator()(const IdleStateData &) const noexcept {
    return "idle state";
}
const char *
StateDataPrinter::operator()(const ReconnectStateData &) const noexcept {
    return "reconnect state";
}

IdleState::IdleState() noexcept : next_state_data{std::monostate{}} {}

int IdleState::perform(Data &data) {
    (void)data;
    return 0;
}

StateData IdleState::get_next_state_data() const noexcept {
    return next_state_data;
}

} // namespace runtime
