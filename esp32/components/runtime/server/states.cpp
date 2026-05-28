#include "server/states.hpp"
#include "errors.hpp"
#include "util.hpp"

using namespace std;

namespace server {
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

} // namespace server