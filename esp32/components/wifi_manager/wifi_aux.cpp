#include "wifi_aux.hpp"

using util::end_bit;
using util::fail_bit;
using util::success_bit;

namespace wifi {
int await_connection(util::I_EventGroup &event_group) {
    auto bits = event_group.wait(success_bit | fail_bit);
    return bits & success_bit ? ESP_OK : ESP_FAIL;
}
} // namespace wifi