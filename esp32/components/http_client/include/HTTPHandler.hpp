#ifndef HTTP_HANDLER_HPP
#define HTTP_HANDLER_HPP

#include "CharVector.hpp"
#include "HTTPClient_WRAP.hpp"
#include "esp_err.h"
#include "httpaux.hpp"

#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

class I_HTTPHandler;

namespace http {
struct cbdata {
    I_HTTPHandler *handler;
    http_event_handle_cb callback;
    int cnt; // check out test_HTTPHandler.cpp
};
} // namespace http

class I_HTTPHandler {
  public:
    virtual ~I_HTTPHandler() noexcept {};
    virtual esp_err_t init(const esp_http_client_config_t *config) = 0;
    virtual void release(CharBuf &out) = 0;
    virtual std::map<std::string, std::string> get_headers() = 0;
    virtual const http::cbdata &get_cbdata() const noexcept = 0;
    virtual I_HTTPClient &get_client() noexcept = 0;
    virtual http::code perform(CharBuf &payload) = 0;
};

enum class HeaderState { first, next };

class HTTPHandler : public I_HTTPHandler {
  public:
    HTTPHandler(std::unique_ptr<I_HTTPClient> &&client);
    esp_err_t init(const esp_http_client_config_t *config) override;
    void release(CharBuf &out) override;
    std::map<std::string, std::string> get_headers() override;
    const http::cbdata &get_cbdata() const noexcept;
    I_HTTPClient &get_client() noexcept override;
    http::code perform(CharBuf &payload) override;
    static esp_err_t callback(esp_http_client_event_t *evt);

  protected:
    CharVector buf{};
    std::map<std::string, std::string> headers{};
    HeaderState header_state = HeaderState::first;
    std::unique_ptr<I_HTTPClient> client{};
    http::cbdata cbdata{};
};

#endif
