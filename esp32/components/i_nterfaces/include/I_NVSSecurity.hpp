#ifndef I_NVS_SECURITY_HPP
#define I_NVS_SECURITY_HPP

#include "esp_err.h"
#include "nvs_flash.h"

class I_NVSSecurity {
    using K = const char;

  public:
    virtual ~I_NVSSecurity() {};
    virtual bool is_flash_encrypted() const noexcept = 0;
    virtual esp_err_t read_security_cfg(nvs_sec_cfg_t *cfg) = 0;
    virtual esp_err_t secure_init_partition(K *name, nvs_sec_cfg_t *cfg) = 0;
    virtual esp_err_t generate_keys(nvs_sec_cfg_t *cfg) = 0;
};

#endif
