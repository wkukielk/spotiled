#ifndef WS_SERVER_HPP
#include "WSServer.hpp"
#endif

#include "util.hpp"
#include <expected>

using namespace std;

namespace {
template <I_HTTPD HTTPD> typename HTTPD::Config get_config() {
    auto config = HTTPD::get_default_config();
    config.server_port = 8000;
    return config;
}

template <I_HTTPD HTTPD>
auto get_payload(httpd_req_t *req) noexcept
    -> std::expected<typename WSServer<HTTPD>::Message, esp_err_t> {
    httpd_ws_frame_t ws_pkt{.type = HTTPD_WS_TYPE_TEXT};
    NEW_UTIL_RETURN(HTTPD::ws_recv_frame(req, &ws_pkt, 0));
    NEW_UTIL_RETURN(ws_pkt.len == 0);

    auto buf = u_ptr<uint8_t[]>{new uint8_t[ws_pkt.len + 1]};
    ws_pkt.payload = buf.get();
    NEW_UTIL_RETURN(HTTPD::ws_recv_frame(req, &ws_pkt, ws_pkt.len));
    buf.get()[ws_pkt.len] = '\0';

    return std::pair{std::move(buf), ws_pkt.len};
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

template <I_HTTPD HTTPD>
WSServer<HTTPD>::WSServer(WSServer &&other)
    : httpd{std::move(other.httpd)}, queue{std::move(other.queue)},
      _client_fd{other._client_fd} {
    other._client_fd = -1;
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

template <I_HTTPD HTTPD>
esp_err_t WSServer<HTTPD>::handler(httpd_req_t *req) noexcept {
    UTIL_RETURN(ensure_single_client(req));
    if (is_ws_handshake(req))
        return ESP_OK;
    auto payload = get_payload<HTTPD>(req);
    UTIL_RETURN(!payload);
    auto ret = _server_ref.get()->queue.send_nowait(std::move(*payload));
    return ret == util::SendStatus::success ? ESP_OK : ESP_FAIL;
}

template <I_HTTPD HTTPD>
esp_err_t WSServer<HTTPD>::ensure_single_client(httpd_req_t *req) noexcept {
    auto *srv = _server_ref.get();
    if (is_ws_handshake(req)) {
        int fd = srv->httpd.req_to_sockfd(req);
        if (srv->_client_fd != -1 &&
            srv->httpd.ws_get_fd_info(srv->_client_fd) ==
                HTTPD_WS_CLIENT_WEBSOCKET) {
            _server_ref.get()->httpd.send_status_response(
                req, "503 Service Unavailable");
            return ESP_FAIL;
        }
        srv->_client_fd = fd;
    }
    return ESP_OK;
}
