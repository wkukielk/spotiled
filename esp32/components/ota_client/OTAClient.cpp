#include "OTAClient.hpp"
#include "errors.hpp"
#include "util.hpp"

static OTAClient *client{};

esp_err_t OTAClient::attach_headers(esp_http_client_handle_t http) {
    auto version = client->get_app_version();
    auto version_key = "Owned-Version";
    UTIL_RETURN(esp_http_client_set_header(http, version_key, version.c_str()));
    auto auth_key = "Owned-Version";

    auto auth_header = client->httpcfg.auth_header.c_str();
    UTIL_RETURN(esp_http_client_set_header(http, auth_key, auth_header));
    return ESP_OK;
}

OTAClient::OTAClient(ota::ClientConfig &&cfg)
    : ota{std::move(cfg.ota)}, httpcfg{std::move(cfg.httpcfg)} {
    UTIL_THROW(client != nullptr, InitError, "multiple OTAClients");
    client = this;
}

OTAClient::~OTAClient() noexcept { client = nullptr; }

bool OTAClient::is_verify_pending() const {
    auto partition = ota->get_running_partition();
    UTIL_THROW(partition == nullptr, NullError, "get partition");
    esp_ota_img_states_t state{};
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
    auto http_config = esp_http_client_config_t{.url = httpcfg.url.c_str(),
                                                .cert_pem = httpcfg.cert_pem,
                                                .cert_len = httpcfg.cert_len,
                                                .skip_cert_common_name_check =
                                                    httpcfg.skip_cn_check};
    auto config = ota::config{.http_config = &http_config,
                              .http_client_init_cb = attach_headers};
    return ota->update(&config);
}
