#include "HTTPD.hpp"

const char *TAG = "HTTPD";

HTTPD::HTTPD(u_ptr<httpd_config_t> config) : config{std::move(config)} {}

HTTPD::~HTTPD() noexcept {
    if (httpd) {
        httpd_stop(httpd);
        UTIL_LOGW("Call HTTP::stop() before destructing HTTPD");
    }
}

esp_err_t HTTPD::start() noexcept { return httpd_start(&httpd, config.get()); }

esp_err_t HTTPD::stop() noexcept {
    auto ret = httpd_stop(httpd);
    httpd = nullptr;
    return ret;
}

esp_err_t HTTPD::register_uri_handler(const httpd_uri_t *uri_handler) {
    return httpd_register_uri_handler(httpd, uri_handler);
}

esp_err_t HTTPD::unregister_uri(const char *uri) {
    return httpd_unregister_uri(httpd, uri);
}

esp_err_t HTTPD::ws_recv_frame(httpd_req_t *req, httpd_ws_frame_t *pkt,
                               size_t max_len) {
    return httpd_ws_recv_frame(req, pkt, max_len);
}

HTTPD::Config HTTPD::get_default_config() noexcept {
    return HTTPD_DEFAULT_CONFIG();
}
