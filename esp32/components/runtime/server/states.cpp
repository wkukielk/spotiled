#include "server/states.hpp"
#include "util.hpp"

using namespace std;

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

IdleState::IdleState() noexcept {}

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

StateFactory::StateFactory() noexcept {}

std::unique_ptr<sm::State<StateData, Data>>
StateFactory::get(StateData &&data) const {
    auto state = std::visit(
        overloaded{
            [&](IdleStateData &&) { return mkuptr<IdleState>(wifi_mgr); },
            [](std::monostate &&) { return u_ptr<IdleState>{nullptr}; }},
        std::move(data));
    UTIL_THROW(!state, NullError, "state factory got monostate");
    return state;
}
