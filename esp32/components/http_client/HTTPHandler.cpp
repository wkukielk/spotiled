#include "HTTPHandler.hpp"
#include "HTTPClient_WRAP.hpp"
#include "httpaux.hpp"
#include <algorithm>
#include <cstring>
#include <stdexcept>

HTTPHandler::HTTPHandler(std::unique_ptr<I_HTTPClient> &&client)
    : client{std::move(client)},
      cbdata{.handler = this, .callback = &HTTPHandler::callback} {}

esp_err_t HTTPHandler::init(const esp_http_client_config_t *config) {
    auto cfg = *config;
    cfg.event_handler = &HTTPHandler::callback;
    cfg.user_data = &cbdata;
    return client->init(&cfg);
}

esp_err_t HTTPHandler::callback(esp_http_client_event_t *evt) {
    auto cbdata = static_cast<http::cbdata *>(evt->user_data);
    auto h = (HTTPHandler *)cbdata->handler;
    switch (evt->event_id) {
    case HTTP_EVENT_ON_HEADER:
        if (h->header_state == HeaderState::first) {
            h->headers.clear();
            h->header_state = HeaderState::next;
            h->buf.clear();
        }
        h->headers.insert({evt->header_key, evt->header_value});
        break;
    case HTTP_EVENT_ON_DATA: {
        if (h->client->is_chunked_response())
            return ESP_FAIL;
        if (!h->buf.is_setup())
            h->buf.setup(h->client->get_content_length() + 1);
        h->buf.extend(evt->data, evt->data_len);
        break;
    }
    case HTTP_EVENT_ON_FINISH:
        h->header_state = HeaderState::first;
        break;
    case HTTP_EVENT_ERROR:
        h->header_state = HeaderState::first;
        break;
    default:
        return ESP_OK;
    }
    return ESP_OK;
}

void HTTPHandler::release(CharBuf &out) { buf.release(out); }

std::map<std::string, std::string> HTTPHandler::get_headers() {
    auto empty = std::map<std::string, std::string>{};
    headers.swap(empty);
    return empty;
}

const http::cbdata &HTTPHandler::get_cbdata() const noexcept { return cbdata; }

I_HTTPClient &HTTPHandler::get_client() noexcept { return *client; }

http::code HTTPHandler::perform(CharBuf &payload) {
    auto err = client->perform();
    if (err != ESP_OK)
        return static_cast<http::code>(err);
    release(payload);
    auto httpcode = client->get_status_code();
    return static_cast<http::code>(httpcode);
}

namespace http {
namespace ContentType {
const char *key = "Content-Type";
const char *json = "application/json";
const char *url = "application/x-www-form-urlencoded";
}; // namespace ContentType
}; // namespace http
