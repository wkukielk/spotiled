#ifndef OTA_CLIENT_HPP
#define OTA_CLIENT_HPP

#include "I_OTA.hpp"

namespace ota {
struct HTTPSConfig {
    std::string url;
    std::string auth_header;
    const char *cert_pem;
    size_t cert_len;
    bool skip_cn_check;
};

struct ClientConfig {
    HTTPSConfig httpcfg;
    std::unique_ptr<I_FullOTA> ota;
};
} // namespace ota

class OTAClient {
  public:
    OTAClient(ota::ClientConfig &&cfg);
    ~OTAClient() noexcept;
    bool is_verify_pending() const;
    esp_err_t mark_app_valid() noexcept;
    std::string get_app_version() const noexcept;
    esp_err_t update() noexcept;
    I_FullOTA &get_ota() noexcept;

  private:
    static esp_err_t attach_headers(esp_http_client_handle_t http);
    std::unique_ptr<I_FullOTA> ota{};
    ota::HTTPSConfig httpcfg{};
};

#endif
