#include "BackendConfigurer.hpp"
#include "OTA.hpp"
#include "data.hpp"
#include "sdkconfig.h"

#if CONFIG_DEV
extern const uint8_t cert_pem_start[] asm("_binary_nginx_crt_start");
extern const uint8_t cert_pem_end[] asm("_binary_nginx_crt_end");
#else
extern const uint8_t cert_pem_start[] asm("_binary_letsencrypt_org_pem_start");
extern const uint8_t cert_pem_end[] asm("_binary_letsencrypt_org_pem_end");
#endif

BackendConfigurer::BackendConfigurer(I_NVS &cfgnvs, I_NVS &eccnvs) noexcept
    : cfgnvs{&cfgnvs}, encnvs{&eccnvs} {}

backend_config_t BackendConfigurer::get() const {
    const auto len = (size_t)(cert_pem_end - cert_pem_start);
    return {.base_url = URL{cfgnvs->read(key::BACKEND_URL)},
            .auth_header{"Bearer " + encnvs->read(key::API_KEY)},
            .cert_pem = (const char *)cert_pem_start,
            .cert_len = len,
            .skip_cn_check = cfgnvs->read(key::SKIP_CN_CHECK) == B::yes};
}
