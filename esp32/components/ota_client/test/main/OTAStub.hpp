#ifndef OTA_STUB_HPP
#define OTA_STUB_HPP

#include "I_OTA.hpp"

class PartitionStub : public I_Partition {
  public:
    esp_err_t
    get_state(esp_ota_img_states_t *ota_state) const noexcept override;
    esp_err_t get_description(esp_app_desc_t *app_desc) const noexcept override;
};

class OTAStub : public I_FullOTA {
  public:
    esp_err_t mark_app_valid(void) noexcept override;
    std::unique_ptr<I_Partition>
    get_running_partition() const noexcept override;
    std::unique_ptr<I_Partition> get_other_partition() const noexcept override;

    esp_err_t update(const ota::config *ota_config) noexcept override;
    esp_err_t begin(const ota::config *ota_config) noexcept override;
    esp_err_t
    get_img_desc(esp_app_desc_t *new_app_info) const noexcept override;
    int get_image_len_read() const noexcept override;
    int get_status_code() const noexcept override;
    esp_err_t perform() noexcept override;
    bool is_all_received() const noexcept override;
    esp_err_t finish() noexcept override;
};

#endif
