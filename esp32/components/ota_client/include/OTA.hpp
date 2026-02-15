#ifndef OTA_HPP
#define OTA_HPP

#include "I_OTA.hpp"
#include <string>

struct ota_config {
    std::string url;
};

class Partition : public I_Partition {
  public:
    Partition(const esp_partition_t *handle) noexcept;
    esp_err_t
    get_state(esp_ota_img_states_t *ota_state) const noexcept override;

  private:
    const esp_partition_t *handle{};
};

class OTA : public I_OTA, public I_HTTPS_OTA {
  public:
    esp_err_t mark_app_valid(void) noexcept override;
    std::unique_ptr<I_Partition> get_running_partition() noexcept override;

    esp_err_t update(const ota::config *ota_config) noexcept override;
    esp_err_t begin(const ota::config *ota_config) noexcept override;
    esp_err_t
    get_img_desc(esp_app_desc_t *new_app_info) const noexcept override;
    int get_image_len_read() const noexcept override;
    int get_status_code() const noexcept override;
    esp_err_t perform() noexcept override;
    bool is_all_received() const noexcept override;
    esp_err_t finish() noexcept override;

  private:
    esp_https_ota_handle_t handle{};
};

#endif
