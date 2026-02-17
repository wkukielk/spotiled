#ifndef OTA_CLIENT_HPP
#define OTA_CLIENT_HPP

#include "I_OTA.hpp"

class OTAClient {
  public:
    OTAClient(std::unique_ptr<FullOTA> &&ota) noexcept;
    bool is_verify_pending() const;
    esp_err_t mark_app_valid() noexcept;
    std::string get_app_version() const noexcept;
    esp_err_t update() noexcept;

  private:
    std::unique_ptr<FullOTA> ota{};
};

#endif
