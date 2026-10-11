#include "nvs.hpp"
#include "errors.hpp"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs_handle.hpp"
#include <cstring>
#include <utility>

static const char *TAG = "NVS";

using namespace std;

namespace _NVS {
u_ptr<nvs::NVSHandle> open(const char *part, nvs_open_mode_t mode,
                           esp_err_t *err) {
    return nvs::open_nvs_handle_from_partition(part, "storage", mode, err);
}

esp_err_t ignore_not_found(esp_err_t err) {
    return err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND ? err : ESP_OK;
}

template <typename T> int read(const char *part, const char *key, T &value) {
    UTIL_THROW(!part || !key, NullError);
    esp_err_t err{};
    auto handle = open(part, NVS_READONLY, &err);
    UTIL_THROW(err, NVSError, "open handle");
    err = handle->get_item(key, value);
    UTIL_THROW(ignore_not_found(err), NVSError, "read error");
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

NVS::NVS(I_NVSSecurity &security) : NVS{NVS_DEFAULT_PART_NAME, security} {}

NVS::NVS(std::string_view part, I_NVSSecurity &security)
    : part{part.data() ? part.data() : ""}, security{security} {}

NVS::~NVS() noexcept {
    if (!part.empty())
        nvs_flash_deinit_partition(this->part.data());
}

NVS::NVS(NVS &&other)
    : part{std::exchange(other.part, {})}, security{other.security} {}

esp_err_t NVS::init() {
    UTIL_THROW(part.empty(), NullError, "nvs part null name");
    auto ret = init_partition();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        if (erase() == ESP_OK)
            UTIL_LOGW("NVS \"%s\" erased", this->part.data());
        ret = init_partition();
    }
    return ret;
}

esp_err_t NVS::erase() {
    auto handle = open(NVS_READWRITE);
    UTIL_RETURN(handle->erase_all());
    UTIL_RETURN(handle->commit());
    return ESP_OK;
}

std::string NVS::read(K key) const {
    UTIL_THROW(!key, NullError);
    auto handle = open(NVS_READONLY);
    if (!handle)
        return {};
    auto size = size_t{};
    auto err = get_string_size(*handle, key, std::ref(size));
    if (err == ESP_ERR_NVS_NOT_FOUND)
        return {};
    auto buf = std::string(size - 1, '\0');
    err = get_string(*handle, key, buf.data(), size);
    return err == ESP_OK ? buf : std::string{};
}

std::string NVS::read_or_throw(K key) const {
    auto buf = read(key);
    UTIL_THROW(buf.empty(), MissingKeyError, "nvs", key);
    return buf;
}

std::string NVS::read(K key, std::string_view dft) const {
    auto buf = read(key);
    if (buf.empty()) {
        buf.resize(dft.size() + 1);
        memcpy(buf.data(), dft.data(), dft.size());
        buf.back() = '\0';
    }
    return buf;
}

void NVS::write(K key, const char *value) {
    UTIL_THROW(!key || !value, NullError);
    auto handle = open(NVS_READWRITE);
    UTIL_THROW(handle->set_string(key, value), NVSError, "write");
    UTIL_THROW(handle->commit(), NVSError, "commit");
}

void NVS::write_time_t(K key, time_t value) {
    _NVS::write(part.data(), key, value);
}

int NVS::read_time_t(K key, time_t &value) const {
    return _NVS::read(part.data(), key, value);
}

u_ptr<nvs::NVSHandle> NVS::open(nvs_open_mode_t mode) const {
    esp_err_t err{};
    auto handle = _NVS::open(part.data(), mode, &err);
    if (mode == NVS_READONLY && err == ESP_ERR_NVS_NOT_FOUND)
        return nullptr;
    UTIL_THROW(err, NVSError, "open");
    return handle;
}

esp_err_t NVS::get_string_size(NVSHandle &h, K key, size_t &size) const {
    auto err = h.get_item_size(nvs::ItemType::SZ, key, std::ref(size));
    UTIL_THROW(_NVS::ignore_not_found(err), NVSError, "get item size");
    return err;
}

esp_err_t NVS::get_string(NVSHandle &h, K key, char *out, size_t len) const {
    auto err = h.get_string(key, out, len);
    UTIL_THROW(_NVS::ignore_not_found(err), NVSError, "get string");
    return err;
}

esp_err_t NVS::init_partition() {
    auto &sec = security.get();
    if (!sec.is_flash_encrypted())
        return nvs_flash_init_partition(this->part.data());
    auto cfg = nvs_sec_cfg_t{};
    auto ret = sec.read_security_cfg(&cfg);
    if (ret == ESP_ERR_NVS_KEYS_NOT_INITIALIZED ||
        ret == ESP_ERR_NVS_CORRUPT_KEY_PART) {
        UTIL_THROW(sec.generate_keys(&cfg), RuntimeError, "nvs keygen fail");
    } else if (ret != ESP_OK)
        return ret;
    return sec.secure_init_partition(part.data(), &cfg);
}

esp_err_t NVSUnencrypted::read_security_cfg(nvs_sec_cfg_t *cfg) {
    (void)cfg;
    UTIL_THROW(1, RuntimeError, "flash not excrypted");
    return ESP_FAIL;
}

esp_err_t NVSUnencrypted::secure_init_partition(const char *name,
                                                nvs_sec_cfg_t *cfg) {
    (void)name;
    (void)cfg;
    UTIL_THROW(1, RuntimeError, "flash not excrypted");
    return ESP_FAIL;
}

esp_err_t NVSUnencrypted::generate_keys(nvs_sec_cfg_t *cfg) {
    (void)cfg;
    UTIL_THROW(1, RuntimeError, "flash not excrypted");
    return ESP_FAIL;
}
