#ifndef OTA_CLIENT_HPP
#include "OTAClient.hpp"
#endif

#include "errors.hpp"

template <typename T_OTA>
esp_err_t OTAClient<T_OTA>::attach_headers(esp_http_client_handle_t http) {
    vTaskDelay(500 / portTICK_PERIOD_MS);
    auto version = _client_ref.get()->get_app_version();
    auto version_key = "Owned-Version";
    UTIL_RETURN(esp_http_client_set_header(http, version_key, version.c_str()));

    auto auth_header = _client_ref.get()->httpcfg.auth_header.c_str();
    UTIL_RETURN(esp_http_client_set_header(http, "Authorization", auth_header));
    return ESP_OK;
}

template <typename T_OTA>
OTAClient<T_OTA>::OTAClient(ota::HTTPSConfig &&cfg)
    : ota{T_OTA{}}, httpcfg{std::move(cfg)} {
    UTIL_THROW(_client_ref.get() != nullptr, InitError, "multiple OTAClients");
    _client_ref.store(this);
}

template <typename T_OTA> OTAClient<T_OTA>::~OTAClient() noexcept {
    _client_ref.drop();
}

template <typename T_OTA>
OTAClient<T_OTA>::OTAClient(OTAClient &&other) noexcept {
    this->ota = std::move(other.ota);
    this->httpcfg = std::move(other.httpcfg);
    _client_ref.store(this);
}

template <typename T_OTA> bool OTAClient<T_OTA>::is_verify_pending() const {
    auto partition = ota.get_running_partition();
    UTIL_THROW(partition == nullptr, NullError, "get partition");
    esp_ota_img_states_t state{};
    auto err = partition->get_state(&state);
    UTIL_THROW(err, RuntimeError, "get partition state");
    return state == ESP_OTA_IMG_PENDING_VERIFY;
}

template <typename T_OTA>
esp_err_t OTAClient<T_OTA>::mark_app_valid() noexcept {
    return ota.mark_app_valid();
}

template <typename T_OTA>
std::string OTAClient<T_OTA>::get_app_version() const noexcept {
    auto partition = ota.get_running_partition();
    esp_app_desc_t desc;
    partition->get_description(&desc);
    return (char *)desc.version;
}

template <typename T_OTA> esp_err_t OTAClient<T_OTA>::update() noexcept {
    auto http_config = esp_http_client_config_t{.url = httpcfg.url.c_str(),
                                                .cert_pem = httpcfg.cert_pem,
                                                .cert_len = httpcfg.cert_len,
                                                .skip_cert_common_name_check =
                                                    httpcfg.skip_cn_check};
    auto config = ota::config{.http_config = &http_config,
                              .http_client_init_cb = attach_headers};
    auto err = ota.update(&config);
    if (err == ESP_OK)
        err = ota.mark_app_valid();
    return err;
}
