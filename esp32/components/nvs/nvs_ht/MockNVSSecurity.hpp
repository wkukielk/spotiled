#ifndef MOCK_NVS_SECURITY_HPP
#define MOCK_NVS_SECURITY_HPP

#include "nvs.hpp"
#include <gmock/gmock.h>

class MockNVSSecurity : public I_NVSSecurity {
  public:
    MOCK_METHOD(bool, is_flash_encrypted, (), (const, noexcept, override));
    MOCK_METHOD(esp_err_t, read_security_cfg, (nvs_sec_cfg_t * cfg),
                (override));
    MOCK_METHOD(esp_err_t, secure_init_partition,
                (const char *name, nvs_sec_cfg_t *cfg), (override));
    MOCK_METHOD(esp_err_t, generate_keys, (nvs_sec_cfg_t * cfg), (override));
};

#endif
