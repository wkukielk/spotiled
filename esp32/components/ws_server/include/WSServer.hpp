#ifndef WS_SERVER_HPP
#define WS_SERVER_HPP

#include "I_HTTPD.hpp"
#include "queue.hpp"
#include "util.hpp"

namespace _wsserver {
enum class ClientNr { FirstClient, SecondClient };
};

template <I_HTTPD HTTPD> class WSServer {
  public:
    WSServer();
    ~WSServer() noexcept;
    WSServer(WSServer &&);
    esp_err_t start() noexcept;
    esp_err_t stop() noexcept;

    using Message = std::pair<std::unique_ptr<uint8_t[]>, size_t>;

    Message receive() { return queue.receive(); };
    Message receive(uint32_t wait) { return queue.receive(wait); };
    Message receive_nowait() { return queue.receive_nowait(); };

  protected:
    static esp_err_t handler(httpd_req_t *req) noexcept;
    static esp_err_t ensure_single_client(httpd_req_t *req) noexcept;
    inline static util::SingletonRef<WSServer<HTTPD> *> _server_ref{};

    HTTPD httpd;
    util::ArrayQueue<uint8_t> queue;
    int _client_fd = -1;
};

namespace ws_server {
extern const char *url;
}

inline bool is_ws_handshake(httpd_req_t *req) {
    return req->method == HTTP_GET;
}

#include "WSServer.tpp"

#endif
