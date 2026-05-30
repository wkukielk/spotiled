#include "HTTPD.hpp"

HTTPD::~HTTPD() noexcept {
    if (httpd) {
        httpd_stop(httpd);
        ESP_LOGW("HTTPD", "Call HTTP::stop() before destructing HTTPD");
    }
}

esp_err_t HTTPD::stop() noexcept {
    auto ret = httpd_stop(httpd);
    httpd = nullptr;
    return ret;
}
