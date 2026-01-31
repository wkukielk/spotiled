#include "time_aux_core.hpp"

struct tm get_tm() {
    time_t now;
    time(&now);
    return *localtime(&now);
}
