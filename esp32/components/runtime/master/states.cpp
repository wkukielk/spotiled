#include "master/states.hpp"
#include "util.hpp"

using namespace std;

namespace master {
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

BaseState::BaseState() noexcept : next_state_data{monostate{}} {}

StateData BaseState::get_next_state_data() const noexcept {
    return next_state_data;
}

IdleState::IdleState(I_WiFiManager &wifi_manager) noexcept
    : wifi_manager{wifi_manager} {}

int IdleState::perform(Data &data) {
    auto ret = ESP_OK;
    std::visit(overloaded{[&](ConnectionErrorData &&) {
                              if (!wifi_manager.get().is_connected())
                                  ret = wifi_manager.get().connect();
                          },
                          [&](ErrorData &&) { ret = -1; },
                          [&](const auto &&) { ret = -2; }},
               std::move(data));
    return ret;
}

} // namespace master
