#include "nvs.hpp"
#include "errors.hpp"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs_handle.hpp"
#include <cstring>

static const char *TAG = "NVS";

namespace _NVS {
u_ptr<nvs::NVSHandle> open(const char *part, nvs_open_mode_t mode,
                           esp_err_t *err) {
    return nvs::open_nvs_handle_from_partition(part, "storage", mode, err);
}

template <typename T> int read(const char *part, const char *key, T &value) {
    UTIL_THROW(!part || !key, NullError);
    esp_err_t err{};
    auto handle = open(part, NVS_READONLY, &err);
    UTIL_THROW(err, NVSError, "open handle");
    err = handle->get_item(key, value);
    auto r = err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND ? err : ESP_OK;
    UTIL_THROW(r, NVSError, "read error");
    return err;
}

template <typename T>
void write(const char *part, const char *key, const T &value) {
    UTIL_THROW(!part || !key, NullError);
    esp_err_t err{};
    auto handle = open(part, NVS_READWRITE, &err);
    UTIL_THROW(err, NVSError, "open");
    UTIL_THROW(handle->set_item(key, value), NVSError, "save");
    UTIL_THROW(err = handle->commit(), NVSError, "commit");
}
} // namespace _NVS

NVS::NVS() : NVS{NVS_DEFAULT_PART_NAME} {}

NVS::NVS(std::string_view part) : part{part.data() ? part.data() : ""} {
    UTIL_THROW(part.empty(), NullError);
    auto ret = nvs_flash_init_partition(this->part.data());
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        if (erase() == ESP_OK)
            UTIL_LOGW("NVS \"%s\" erased", this->part.data());
        ret = nvs_flash_init_partition(this->part.data());
    }
    UTIL_THROW(ret, InitError, "NVS");
}

NVS::~NVS() { nvs_flash_deinit(); }

esp_err_t NVS::erase() {
    esp_err_t err{};
    auto handle = open(NVS_READWRITE, &err);
    UTIL_RETURN(handle->erase_all());
    UTIL_RETURN(handle->commit());
    return ESP_OK;
}

int NVS::read(const char *key, CharBuf &buf) const {
    UTIL_THROW(!key, NullError);
    esp_err_t err{};
    auto handle = open(NVS_READONLY, &err);
    UTIL_THROW(err, NVSError, "open");
    size_t size{};
    err = handle->get_item_size(nvs::ItemType::SZ, key, std::ref(size));
    UTIL_RETURN(err == ESP_ERR_NVS_NOT_FOUND ? err : ESP_OK);
    UTIL_THROW(err, NVSError, "check size");
    buf.resize(size);
    err = handle->get_string(key, buf.data(), buf.size());
    auto r = err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND ? err : ESP_OK;
    UTIL_THROW(r, NVSError, "read");
    return err;
}

CharBuf NVS::read(K key, std::string_view dft) const {
    CharBuf buf{};
    if (read(key, buf) != ESP_OK) {
        buf.resize(dft.size() + 1);
        memcpy(buf.data(), dft.data(), dft.size());
        buf.back() = '\0';
    }
    return buf;
}

void NVS::write(const char *key, const char *value) {
    UTIL_THROW(!key || !value, NullError);
    esp_err_t err;
    auto handle = open(NVS_READWRITE, &err);
    UTIL_THROW(err, NVSError, "open");
    UTIL_THROW(handle->set_string(key, value), NVSError, "write");
    UTIL_THROW(handle->commit(), NVSError, "commit");
}

void NVS::write_time_t(const char *key, time_t value) {
    _NVS::write(part.data(), key, value);
}

int NVS::read_time_t(const char *key, time_t &value) const {
    return _NVS::read(part.data(), key, value);
}

u_ptr<nvs::NVSHandle> NVS::open(nvs_open_mode_t mode, esp_err_t *err) const {
    return _NVS::open(part.data(), mode, err);
}
