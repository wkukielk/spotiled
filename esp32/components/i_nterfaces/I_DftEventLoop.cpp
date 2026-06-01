#include "I_DftEventLoop.hpp"

std::optional<DftEventLoop> DftEventLoop::instance{};

void DftEventLoop::create() { instance.emplace(); }
