#ifndef MOCK_HTTP_HANDLER_HPP
#define MOCK_HTTP_HANDLER_HPP

#include "HTTPHandler.hpp"
#include "MockHTTPClient.hpp"
#include <gmock/gmock.h>

class MockHTTPHandler : public I_HTTPHandler {
  public:
    using StringMap = std::map<std::string, std::string>;
    MOCK_METHOD(esp_err_t, init, (const esp_http_client_config_t *),
                (override));
    MOCK_METHOD(int, release, (straux::buf<char> & out), (override));
    MOCK_METHOD(StringMap, get_headers, (), (override));
    MOCK_METHOD(const http::cbdata &, get_cbdata, (),
                (const, noexcept, override));
    I_HTTPClient &get_client() noexcept override { return client; };
    MOCK_METHOD(http::code, perform, (straux::buf<char> & payload), (override));

  public:
    MockHTTPClient client{};
};

#endif
