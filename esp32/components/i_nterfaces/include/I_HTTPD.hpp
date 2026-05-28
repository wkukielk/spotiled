#ifndef I_NTERFACES_HTTPD_HPP
#define I_NTERFACES_HTTPD_HPP

#include "esp_err.h"

#ifdef CONFIG_IDF_TARGET_LINUX
#define CONFIG_HTTPD_MAX_URI_LEN 512
#define CONFIG_HTTPD_ERR_RESP_NO_DELAY 1
#define CONFIG_HTTPD_PURGE_BUF_LEN 32
#define CONFIG_HTTPD_WS_SUPPORT 1
#define CONFIG_HTTPD_SERVER_EVENT_POST_TIMEOUT 2000
#define CONFIG_ESP_HTTPS_OTA_EVENT_POST_TIMEOUT 2000
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "esp_http_server.h"
#pragma GCC diagnostic pop

#include <concepts>
#include <memory>

template <typename T>
concept I_HTTPD_CONFIG =
    requires(T config) { config.server_port = uint16_t{}; };

template <typename T>
concept I_HTTPD =
    std::constructible_from<T, std::unique_ptr<typename T::Config>> &&
    requires(T httpd) {
        typename T::Config;
        requires I_HTTPD_CONFIG<typename T::Config>;

        { httpd.start() } -> std::same_as<esp_err_t>;
        { httpd.stop() } -> std::same_as<esp_err_t>;
        {
            httpd.register_uri_handler((const httpd_uri_t *){})
        } -> std::same_as<esp_err_t>;
        { httpd.unregister_uri((const char *){}) } -> std::same_as<esp_err_t>;

        {
            T::ws_recv_frame((httpd_req_t *){}, (httpd_ws_frame_t *){},
                             size_t{})
        } -> std::same_as<esp_err_t>;
        { T::get_default_config() } -> std::same_as<typename T::Config>;
    };

#endif
