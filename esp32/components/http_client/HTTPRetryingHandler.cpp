#include "HTTPRetryingHandler.hpp"
#include "errors.hpp"
#include "esp_log.h"
#include "global.hpp"
#include "time_aux.hpp"

static esp_http_client_config_t config{.url = "http://localhost:8888/",
                                       .method = HTTP_METHOD_GET,
                                       .transport_type =
                                           HTTP_TRANSPORT_OVER_TCP};

HTTPRetryingHandler::HTTPRetryingHandler(std::unique_ptr<I_HTTPClient> &&client)
    : HTTPHandler{std::move(client)} {
    UTIL_THROW(init(&config), InitError, "HTTPRetryingHandler");
}

http::code HTTPRetryingHandler::perform(CharBuf &buff) {
    static const uint8_t retries = 5;
    uint8_t retry = retries;
    auto code = http::code{};
    do {
        code = HTTPHandler::perform(buff);
        ESP_LOGD("HTTPRetryingHandler", "httpcode: %d\n", code);
        if (http::is_connection_error(code))
            time().delay_ms(3000);
    } while (http::is_connection_error(code) && --retry != 0);
    return code;
}
