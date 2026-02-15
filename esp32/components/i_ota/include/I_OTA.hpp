#ifndef I_OTA_HPP
#define I_OTA_HPP

#include "esp_https_ota.h"
#include <memory>

class I_Partition {
  public:
    virtual esp_err_t
    get_state(esp_ota_img_states_t *ota_state) const noexcept = 0;
};

class I_OTA {
  public:
    virtual esp_err_t mark_app_valid(void) noexcept = 0;
    virtual std::unique_ptr<I_Partition> get_running_partition() noexcept = 0;
};

namespace ota {
using config = esp_https_ota_config_t;
}

class I_HTTPS_OTA {
  public:
    virtual esp_err_t update(const ota::config *ota_config) noexcept = 0;
    virtual esp_err_t begin(const ota::config *ota_config) noexcept = 0;
    virtual esp_err_t
    get_img_desc(esp_app_desc_t *new_app_info) const noexcept = 0;
    virtual int get_image_len_read() const noexcept = 0;
    virtual int get_status_code() const noexcept = 0;
    virtual esp_err_t perform() noexcept = 0;
    virtual bool is_all_received() const noexcept = 0;
    virtual esp_err_t finish() noexcept = 0;
};

#endif
