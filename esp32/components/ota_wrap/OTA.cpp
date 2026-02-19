#include "OTA.hpp"
#include "esp_https_ota.h"
#include "esp_ota_ops.h"

Partition::Partition(const esp_partition_t *handle) noexcept : handle{handle} {}

esp_err_t Partition::get_state(esp_ota_img_states_t *ota_state) const noexcept {
    return esp_ota_get_state_partition(handle, ota_state);
}

std::unique_ptr<I_Partition> OTA::get_running_partition() const noexcept {
    return std::make_unique<Partition>(esp_ota_get_running_partition());
}

std::unique_ptr<I_Partition> OTA::get_other_partition() const noexcept {
    return std::make_unique<Partition>(esp_ota_get_last_invalid_partition());
}

esp_err_t OTA::mark_app_valid(void) noexcept {
    return esp_ota_mark_app_valid_cancel_rollback();
}

esp_err_t OTA::update(const ota::config *ota_config) noexcept {
    return esp_https_ota(ota_config);
}

esp_err_t OTA::begin(const ota::config *ota_config) noexcept {
    return esp_https_ota_begin(ota_config, &handle);
}

esp_err_t OTA::get_img_desc(esp_app_desc_t *new_app_info) const noexcept {
    return esp_https_ota_get_img_desc(handle, new_app_info);
}

int OTA::get_image_len_read() const noexcept {
    return esp_https_ota_get_image_len_read(handle);
}

int OTA::get_status_code() const noexcept {
    return esp_https_ota_get_status_code(handle);
}

esp_err_t OTA::perform() noexcept { return esp_https_ota_perform(handle); }

bool OTA::is_all_received() const noexcept {
    return esp_https_ota_is_complete_data_received(handle);
}

esp_err_t OTA::finish() noexcept { return esp_https_ota_finish(handle); }
