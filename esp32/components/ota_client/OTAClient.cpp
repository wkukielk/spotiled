#include "OTAClient.hpp"
#include "errors.hpp"
#include "util.hpp"

static OTAClient *client{};

static esp_err_t attach_headers(esp_http_client_handle_t http) {
    (void)http;
    (void)client;
    return ESP_OK;
}

OTAClient::OTAClient(std::unique_ptr<FullOTA> &&ota) noexcept
    : ota{std::move(ota)} {}

bool OTAClient::is_verify_pending() const {
    auto partition = ota->get_running_partition();
    esp_ota_img_states_t state;
    auto err = partition->get_state(&state);
    UTIL_THROW(err, RuntimeError, "get partition state");
    return state == ESP_OTA_IMG_PENDING_VERIFY;
}

esp_err_t OTAClient::mark_app_valid() noexcept { return ota->mark_app_valid(); }

std::string OTAClient::get_app_version() const noexcept {
    esp_app_desc_t desc;
    ota->get_img_desc(&desc);
    return desc.version;
}

esp_err_t OTAClient::update() noexcept {
    auto http_config = esp_http_client_config_t{};
    auto config = ota::config{.http_config = &http_config,
                              .http_client_init_cb = attach_headers};
    return ota->update(&config);
}
