#include "MockHTTPD.hpp"

thread_local MockHTTPD *MockHTTPD::instance = nullptr;

MockHTTPD::MockHTTPD(u_ptr<Config> config) {
    (void)config;
    MockHTTPD::instance = this;
}

esp_err_t MockHTTPD::ws_recv_frame(httpd_req_t *req, httpd_ws_frame_t *pkt,
                                   size_t max_len) {
    return MockHTTPD::instance->ws_recv_frame_impl(req, pkt, max_len);
}

MockHTTPD::Config MockHTTPD::get_default_config() noexcept {
    return MockHTTPD::Config{};
}
