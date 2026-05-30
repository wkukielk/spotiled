#ifndef WRAPPERS_HTTPD_HPP
#define WRAPPERS_HTTPD_HPP

#include "esp_http_server.h"
#include "util.hpp"

class HTTPD {
  public:
    using Config = httpd_config_t;

    explicit HTTPD(httpd_config_t config) : config{std::move(config)} {};
    ~HTTPD() noexcept;
    esp_err_t start() noexcept { return httpd_start(&httpd, &config); }
    esp_err_t stop() noexcept;
    esp_err_t register_uri_handler(const httpd_uri_t *uri_handler) {
        return httpd_register_uri_handler(httpd, uri_handler);
    }
    esp_err_t unregister_uri(const char *uri) {
        return httpd_unregister_uri(httpd, uri);
    }
    static esp_err_t ws_recv_frame(httpd_req_t *req, httpd_ws_frame_t *pkt,
                                   size_t max_len) {
        return httpd_ws_recv_frame(req, pkt, max_len);
    }
    static Config get_default_config() noexcept {
        return HTTPD_DEFAULT_CONFIG();
    }

  private:
    httpd_config_t config{};
    httpd_handle_t httpd{};
};

#endif
