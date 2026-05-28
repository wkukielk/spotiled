#include "OTAStub.hpp"
#include <cstring>

esp_err_t
PartitionStub::get_state(esp_ota_img_states_t *ota_state) const noexcept {
    *ota_state = ESP_OTA_IMG_VALID;
    return ESP_OK;
}

esp_err_t
PartitionStub::get_description(esp_app_desc_t *app_desc) const noexcept {
    strcpy(app_desc->version, "version");
    return ESP_OK;
}

esp_err_t OTAStub::mark_app_valid(void) noexcept { return ESP_OK; }
std::unique_ptr<I_Partition> OTAStub::get_running_partition() const noexcept {
    return std::make_unique<PartitionStub>();
}
std::unique_ptr<I_Partition> OTAStub::get_other_partition() const noexcept {
    return std::make_unique<PartitionStub>();
}

esp_err_t OTAStub::update(const ota::config *ota_config) noexcept {
    auto http = esp_http_client_init(ota_config->http_config);
    return ota_config->http_client_init_cb(http);
}

esp_err_t OTAStub::begin(const ota::config *ota_config) noexcept {
    (void)ota_config;
    return ESP_OK;
}

esp_err_t OTAStub::get_img_desc(esp_app_desc_t *new_app_info) const noexcept {
    (void)new_app_info;
    return ESP_OK;
}

int OTAStub::get_image_len_read() const noexcept { return ESP_OK; }
int OTAStub::get_status_code() const noexcept { return ESP_OK; }
esp_err_t OTAStub::perform() noexcept { return ESP_OK; }
bool OTAStub::is_all_received() const noexcept { return ESP_OK; }
esp_err_t OTAStub::finish() noexcept { return ESP_OK; }