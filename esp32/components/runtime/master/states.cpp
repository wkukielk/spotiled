#include "master/states.hpp"
#include "util.hpp"

using namespace std;

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

BaseState::BaseState(runtime_t &runtime) noexcept
    : runtime{runtime}, next_state_data{monostate{}} {}

StateData BaseState::get_next_state_data() const noexcept {
    return next_state_data;
}

IdleState::IdleState(util::Notifiable<runtime::Data> &runtime) noexcept
    : BaseState{runtime} {}

int IdleState::perform(Data &data) {
    auto ret = ESP_OK;
    std::visit(overloaded{[&](ConnectionErrorData &&) {
                              next_state_data = ReconnectStateData{};
                          },
                          [&](ErrorData &&) { ret = -1; },
                          [&](const auto &&) { ret = -2; }},
               std::move(data));
    return ret;
}

ReconnectState::ReconnectState(runtime_t &runtime) noexcept
    : BaseState{runtime} {}

int ReconnectState::perform(Data &data) {
    auto ret = ESP_OK;
    std::visit(overloaded{[&](ConnectionErrorData &&) {
                              next_state_data = IdleStateData{};
                          },
                          [&](const auto &&) { ret = -1; }},
               std::move(data));
    return ret;
}

} // namespace runtime
