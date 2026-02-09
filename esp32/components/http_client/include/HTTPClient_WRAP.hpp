#ifndef HTTP_CLIENT_WRAP
#define HTTP_CLIENT_WRAP

#include <memory>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "esp_http_client.h"
#pragma GCC diagnostic pop

class I_HTTPClient {
  public:
    virtual ~I_HTTPClient() noexcept {};
    virtual esp_err_t init(const esp_http_client_config_t *config) = 0;
    virtual bool is_chunked_response() = 0;
    virtual int64_t get_content_length() = 0;
    virtual esp_err_t set_url(const char *url) = 0;
    virtual esp_err_t set_method(esp_http_client_method_t method) = 0;
    virtual esp_err_t set_header(const char *key, const char *value) = 0;
    virtual esp_err_t perform() = 0;
    virtual int get_status_code() = 0;
    virtual esp_err_t set_authtype(esp_http_client_auth_type_t auth_type) = 0;
    virtual esp_err_t set_username(const char *username) = 0;
    virtual esp_err_t set_password(const char *password) = 0;
    virtual esp_err_t set_post_field(const char *data, int len) = 0;
};

struct HTTPClientDeleter {
    void operator()(esp_http_client_handle_t handle) const;
};

class HTTPClient_WRAP : public I_HTTPClient {
  public:
    esp_err_t init(const esp_http_client_config_t *config) override;
    bool is_chunked_response() override;
    int64_t get_content_length() override;
    esp_err_t set_url(const char *url) override;
    esp_err_t set_method(esp_http_client_method_t method) override;
    esp_err_t set_header(const char *key, const char *value) override;
    esp_err_t perform() override;
    int get_status_code() override;
    esp_err_t set_authtype(esp_http_client_auth_type_t auth_type) override;
    esp_err_t set_username(const char *username) override;
    esp_err_t set_password(const char *password) override;
    esp_err_t set_post_field(const char *data, int len) override;

  private:
    std::unique_ptr<esp_http_client, HTTPClientDeleter> handle{};
};

#endif
