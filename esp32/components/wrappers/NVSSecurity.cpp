#include "NVSSecurity.hpp"
#include "errors.hpp"
#include "esp_attr.h"
#include "nvs_flash.h"
#include "util.hpp"

bool NVSSecurity::is_flash_encrypted() const noexcept {
#ifdef CONFIG_NVS_ENCRYPTION
    return true;
#else
    return false;
#endif
}

esp_err_t NVSSecurity::read_security_cfg(nvs_sec_cfg_t *cfg) {
    auto *key_part = find_keys_partition();
    return nvs_flash_read_security_cfg(key_part, cfg);
}

esp_err_t NVSSecurity::secure_init_partition(const char *name,
                                             nvs_sec_cfg_t *cfg) {
    return nvs_flash_secure_init_partition(name, cfg);
}

esp_err_t NVSSecurity::generate_keys(nvs_sec_cfg_t *cfg) {
    auto *key_part = find_keys_partition();
    return nvs_flash_generate_keys(key_part, cfg);
}

const esp_partition_t *NVSSecurity::find_keys_partition() {
    const esp_partition_t *key_part = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS_KEYS, NULL);
    UTIL_THROW(key_part == NULL, NullError);
    return key_part;
}
