#ifndef WS_SERVER_HPP
#include "WSServer.hpp"
#endif

#include <util.hpp>

using namespace std;

namespace {
template <I_HTTPD HTTPD> u_ptr<typename HTTPD::Config> get_config() {
    auto config = HTTPD::get_default_config();
    config.server_port = 8000;
    return make_unique<typename HTTPD::Config>(config);
}
} // namespace

template <I_HTTPD HTTPD> esp_err_t WSServer<HTTPD>::start() noexcept {
    return httpd.start();
}

template <I_HTTPD HTTPD> esp_err_t WSServer<HTTPD>::stop() noexcept {
    return httpd.stop();
}

template <I_HTTPD HTTPD>
WSServer<HTTPD>::WSServer() : httpd{get_config<HTTPD>()} {}
