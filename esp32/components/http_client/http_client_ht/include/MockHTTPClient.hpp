#ifndef MOCK_HTTP_CLIENT
#define MOCK_HTTP_CLIENT

#include "HTTPClient_WRAP.hpp"
#include <gmock/gmock.h>

class MockHTTPClient : public I_HTTPClient {
  public:
    MOCK_METHOD(esp_err_t, init, (const esp_http_client_config_t *),
                (override));
    MOCK_METHOD(bool, is_chunked_response, (), (override));
    MOCK_METHOD(int64_t, get_content_length, (), (override));
    MOCK_METHOD(esp_err_t, set_url, (const char *url), (override));
    MOCK_METHOD(esp_err_t, set_method, (esp_http_client_method_t method),
                (override));
    MOCK_METHOD(esp_err_t, set_header, (const char *key, const char *value),
                (override));
    MOCK_METHOD(esp_err_t, perform, (), (override));
    MOCK_METHOD(int, get_status_code, (), (override));
    MOCK_METHOD(esp_err_t, set_authtype,
                (esp_http_client_auth_type_t auth_type), (override));
    MOCK_METHOD(esp_err_t, set_username, (const char *username), (override));
    MOCK_METHOD(esp_err_t, set_password, (const char *password), (override));
    MOCK_METHOD(esp_err_t, set_post_field, (const char *data, int len),
                (override));
};

#endif
