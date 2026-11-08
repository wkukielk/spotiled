#ifndef OTA_CLIENT_HPP
#define OTA_CLIENT_HPP

#include "I_OTA.hpp"
#include <atomic>
#include <optional>
#include <string>

namespace ota {
struct HTTPSConfig {
    std::string url;
    std::string auth_header;
    const char *cert_pem;
    size_t cert_len;
    bool skip_cn_check;
    // Don't blacklist the rolled-back version (forced update).
    bool skip_blacklist = false;
};
} // namespace ota

template <typename T_OTA> class OTAClient {
  public:
    OTAClient(ota::HTTPSConfig &&cfg);
    ~OTAClient() noexcept;
    OTAClient(OTAClient &&) noexcept;
    bool is_verify_pending() const;
    esp_err_t mark_app_valid() noexcept;
    std::string get_app_version() const noexcept;
    // Version that was rolled back from; nullopt if none.
    std::optional<std::string> get_invalid_version() const noexcept;
    // Update URL, blacklisting the invalid version so it isn't offered again.
    std::string get_update_url() const;
    esp_err_t update() noexcept;

  protected:
    static esp_err_t attach_headers(esp_http_client_handle_t http);
    inline static util::SingletonRef<OTAClient<T_OTA> *> _client_ref{};

    T_OTA ota;
    ota::HTTPSConfig httpcfg{};
};

#include "OTAClient.tpp"

#endif
