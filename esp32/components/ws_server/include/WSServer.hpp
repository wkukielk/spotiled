#ifndef WS_SERVER_HPP
#define WS_SERVER_HPP

#include "I_HTTPD.hpp"
#include "queue.hpp"
#include "util.hpp"

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
    static esp_err_t handler(httpd_req_t *req);
    inline static util::SingletonRef<WSServer<HTTPD> *> _server_ref{};

    HTTPD httpd;
    util::ArrayQueue<uint8_t> queue;
};

namespace ws_server {
extern const char *url;
}

#include "WSServer.tpp"

#endif
