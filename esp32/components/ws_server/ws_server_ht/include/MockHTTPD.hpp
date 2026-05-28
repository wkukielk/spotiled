#ifndef MOCK_PARTITION_HPP
#define MOCK_PARTITION_HPP

#define CONFIG_HTTPD_MAX_URI_LEN 512
#define CONFIG_HTTPD_ERR_RESP_NO_DELAY 1
#define CONFIG_HTTPD_PURGE_BUF_LEN 32
#define CONFIG_HTTPD_WS_SUPPORT 1
#define CONFIG_HTTPD_SERVER_EVENT_POST_TIMEOUT 2000

#include "I_HTTPD.hpp"
#include "util.hpp"
#include <gmock/gmock.h>

struct HTTPDConfig {
    uint16_t server_port;
};

class MockHTTPD {
  public:
    using Config = HTTPDConfig;

    MockHTTPD(u_ptr<HTTPDConfig> config);
    MOCK_METHOD(esp_err_t, start, (), (noexcept));
    MOCK_METHOD(esp_err_t, stop, (), (noexcept));
    MOCK_METHOD(esp_err_t, register_uri_handler,
                (const httpd_uri_t *uri_handler));
    MOCK_METHOD(esp_err_t, unregister_uri, (const char *uri));

    MOCK_METHOD(esp_err_t, ws_recv_frame_impl,
                (httpd_req_t * req, httpd_ws_frame_t *pkt, size_t max_len));

    static esp_err_t ws_recv_frame(httpd_req_t *req, httpd_ws_frame_t *pkt,
                                   size_t max_len);
    static Config get_default_config() noexcept;

    static thread_local MockHTTPD *instance;
};

#endif
