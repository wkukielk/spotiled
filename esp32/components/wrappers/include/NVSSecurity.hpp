#ifndef NVS_SECURITY_HPP
#define NVS_SECURITY_HPP

#include "I_NVSSecurity.hpp"

class NVSSecurity : public I_NVSSecurity {
  public:
    bool is_flash_encrypted() const noexcept override;
    esp_err_t read_security_cfg(nvs_sec_cfg_t *cfg) override;
    esp_err_t secure_init_partition(const char *name,
                                    nvs_sec_cfg_t *cfg) override;
    esp_err_t generate_keys(nvs_sec_cfg_t *cfg) override;

  private:
    const esp_partition_t *find_keys_partition();
};

#endif
