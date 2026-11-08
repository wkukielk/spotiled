#ifndef I_OTA_HPP
#define I_OTA_HPP

#include "util.hpp"

IGNORE_UNUSED_PUSH
#include "esp_https_ota.h"
IGNORE_UNUSED_POP
#include <memory>

class I_Partition {
  public:
    virtual ~I_Partition() noexcept {};
    virtual esp_err_t
    get_state(esp_ota_img_states_t *ota_state) const noexcept = 0;
    virtual esp_err_t
    get_description(esp_app_desc_t *app_desc) const noexcept = 0;
};

class I_OTA {
  public:
    virtual ~I_OTA() noexcept {};
    virtual esp_err_t mark_app_valid() noexcept = 0;
    virtual std::unique_ptr<I_Partition>
    get_running_partition() const noexcept = 0;

    virtual std::unique_ptr<I_Partition>
    get_other_partition() const noexcept = 0;
};

namespace ota {
using config = esp_https_ota_config_t;
}

class I_HTTPS_OTA {
  public:
    virtual ~I_HTTPS_OTA() noexcept {};
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

class I_FullOTA : public I_HTTPS_OTA, public I_OTA {};

#endif
