#include "BackendConfigurer.hpp"
#include "OTA.hpp"
#include "data.hpp"

extern const uint8_t
    letsencrypt_org_pem_start[] asm("_binary_letsencrypt_org_pem_start");
extern const uint8_t
    letsencrypt_org_pem_end[] asm("_binary_letsencrypt_org_pem_end");

BackendConfigurer::BackendConfigurer(I_NVS &cfgnvs) noexcept
    : cfgnvs{&cfgnvs} {}

backend_config_t BackendConfigurer::get() const {
    auto len = (size_t)(letsencrypt_org_pem_end - letsencrypt_org_pem_start);
    return {.base_url = URL{cfgnvs->read(key::BACKEND_HOST)},
            .cert_pem = (const char *)letsencrypt_org_pem_start,
            .cert_len = len,
            .skip_cn_check = cfgnvs->read(key::SKIP_CN_CHECK) == B::yes};
}
