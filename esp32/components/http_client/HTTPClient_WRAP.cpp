#include "HTTPClient_WRAP.hpp"
#include "errors.hpp"

void HTTPClientDeleter::operator()(esp_http_client_handle_t handle) const {
    esp_http_client_cleanup(handle);
}

esp_err_t HTTPClient_WRAP::init(const esp_http_client_config_t *config) {
    handle = std::unique_ptr<esp_http_client, HTTPClientDeleter>(
        esp_http_client_init(config), HTTPClientDeleter{});
    return handle.get() != nullptr ? ESP_OK : ESP_FAIL;
}

bool HTTPClient_WRAP::is_chunked_response() {
    return esp_http_client_is_chunked_response(handle.get());
}

int64_t HTTPClient_WRAP::get_content_length() {
    return esp_http_client_get_content_length(handle.get());
}

esp_err_t HTTPClient_WRAP::set_url(const char *url) {
    return esp_http_client_set_url(handle.get(), url);
}

esp_err_t HTTPClient_WRAP::set_method(esp_http_client_method_t method) {
    return esp_http_client_set_method(handle.get(), method);
}

esp_err_t HTTPClient_WRAP::set_header(const char *key, const char *value) {
    return esp_http_client_set_header(handle.get(), key, value);
}

esp_err_t HTTPClient_WRAP::perform() {
    return esp_http_client_perform(handle.get());
}

int HTTPClient_WRAP::get_status_code() {
    return esp_http_client_get_status_code(handle.get());
}

esp_err_t HTTPClient_WRAP::set_authtype(esp_http_client_auth_type_t auth_type) {
    return esp_http_client_set_authtype(handle.get(), auth_type);
}

esp_err_t HTTPClient_WRAP::set_username(const char *username) {
    return esp_http_client_set_username(handle.get(), username);
}

esp_err_t HTTPClient_WRAP::set_password(const char *password) {
    return esp_http_client_set_password(handle.get(), password);
}

esp_err_t HTTPClient_WRAP::set_post_field(const char *data, int len) {
    return esp_http_client_set_post_field(handle.get(), data, len);
}
