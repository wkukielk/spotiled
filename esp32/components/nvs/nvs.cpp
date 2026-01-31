#include "nvs.hpp"
#include "errors.hpp"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs_handle.hpp"
#include "util.hpp"

static const char *TAG = "NVS";
const char *nvs_ns = "storage";

namespace _NVS {
template <typename T> int read(const char *key, T &value) {
    esp_err_t err;
    std::unique_ptr<nvs::NVSHandle> handle =
        nvs::open_nvs_handle(nvs_ns, NVS_READONLY, &err);
    UTIL_THROW(err, NVSError{"open handle"});
    err = handle->get_item(key, value);
    bool is_error = err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND;
    UTIL_THROW(is_error, NVSError{"read error"});
    return err;
}

template <typename T> void write(const char *key, const T &value) {
    esp_err_t err;
    auto handle = nvs::open_nvs_handle(nvs_ns, NVS_READWRITE, &err);
    UTIL_THROW(err, NVSError{"open handle"});
    UTIL_THROW(handle->set_item(key, value), NVSError{"save"});
    UTIL_THROW(err = handle->commit(), NVSError{"commit"});
}
} // namespace _NVS

NVS::NVS() : NVS{"nvs"} {}

NVS::NVS(const char *partition_label) {
    auto ret = nvs_flash_init_partition(partition_label);
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        if (erase() == ESP_OK)
            UTIL_LOGW("NVS \"%s\" erased", partition_label);
        ret = nvs_flash_init();
    }
    UTIL_THROW(ret, InitError{"NVS"});
}

NVS::~NVS() { nvs_flash_deinit(); }

esp_err_t NVS::erase() {
    esp_err_t err{};
    auto handle = nvs::open_nvs_handle(nvs_ns, NVS_READWRITE, &err);
    UTIL_RETURN(handle->erase_all());
    UTIL_RETURN(handle->commit());
    return ESP_OK;
}

int NVS::read(const char *key, straux::cvector<char> &buf) {
    esp_err_t err{};
    auto handle = nvs::open_nvs_handle(nvs_ns, NVS_READONLY, &err);
    UTIL_THROW(err, NVSError{"open"});
    size_t size{};
    err = handle->get_item_size(nvs::ItemType::SZ, key, std::ref(size));
    UTIL_RETURN(err == ESP_ERR_NVS_NOT_FOUND);
    UTIL_THROW(err, NVSError{"check size"});
    buf.resize(size);
    err = handle->get_string(key, buf.data(), buf.size());
    UTIL_THROW(err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND, NVSError{"read"});
    return err;
}

void NVS::write(const char *key, const char *value) {
    esp_err_t err;
    auto handle = nvs::open_nvs_handle(nvs_ns, NVS_READWRITE, &err);
    UTIL_THROW(err, NVSError{"open"});
    UTIL_THROW(handle->set_string(key, value), NVSError{"write"});
    UTIL_THROW(handle->commit(), NVSError{"commit"});
}

void NVS::write_time_t(const char *key, time_t value) {
    _NVS::write(key, value);
}

int NVS::read_time_t(const char *key, time_t &value) {
    return _NVS::read(key, value);
}