#include "runtime.hpp"

Runtime::Runtime(u_ptr<I_WiFiManager> wifi_mgr)
    : wifi_mgr{std::move(wifi_mgr)} {}

void Runtime::run() {}
