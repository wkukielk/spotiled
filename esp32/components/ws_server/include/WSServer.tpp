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

template <I_HTTPD HTTPD>
WSServer<HTTPD>::WSServer() : httpd{get_config<HTTPD>()}, queue{4} {
    UTIL_THROW(_server_ref.get() != nullptr, InitError, "multiple WSServers");
    _server_ref.store(this);
}

template <I_HTTPD HTTPD> WSServer<HTTPD>::~WSServer() noexcept {
    _server_ref.drop();
}

template <I_HTTPD HTTPD> WSServer<HTTPD>::WSServer(WSServer &&other) {
    this->httpd = std::move(other.httpd);
    this->queue = std::move(other.queue);
    _server_ref.store(this);
}

template <I_HTTPD HTTPD> esp_err_t WSServer<HTTPD>::start() noexcept {
    const httpd_uri_t handler_cfg = {
        .uri = ws_server::url,
        .method = HTTP_GET,
        .handler = handler,
        .user_ctx = NULL,
        .is_websocket = true,
    };
    UTIL_RETURN(httpd.start());
    return httpd.register_uri_handler(&handler_cfg);
}

template <I_HTTPD HTTPD> esp_err_t WSServer<HTTPD>::stop() noexcept {
    UTIL_RETURN(httpd.unregister_uri(ws_server::url));
    return httpd.stop();
}

template <I_HTTPD HTTPD> esp_err_t WSServer<HTTPD>::handler(httpd_req_t *req) {
    httpd_ws_frame_t ws_pkt{.type = HTTPD_WS_TYPE_TEXT};
    UTIL_RETURN(httpd_ws_recv_frame(req, &ws_pkt, 0));
    UTIL_RETURN(ws_pkt.len == 0);

    auto buf = u_ptr<uint8_t[]>{new uint8_t[ws_pkt.len + 1]};
    ws_pkt.payload = buf.get();
    UTIL_RETURN(httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len));
    buf.get()[ws_pkt.len] = '\0';

    auto elem = std::pair{std::move(buf), ws_pkt.len};
    auto ret = _server_ref.get()->queue.send_nowait(std::move(elem));
    UTIL_RETURN(ret != util::SendStatus::success);
    return ESP_OK;
}
