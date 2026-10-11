#include "BackendConfigurer.hpp"
#include "OTA.hpp"
#include "data.hpp"
#include "sdkconfig.h"

#if CONFIG_DEV
#define BINARY_CERT "nginx_pem"
#elif CONFIG_RELEASE
#define BINARY_CERT "isrg_root_x2_pem"
#else
#define BINARY_CERT "cert_stub_pem"
#endif

extern const uint8_t cert_pem_start[] asm("_binary_" BINARY_CERT "_start");
extern const uint8_t cert_pem_end[] asm("_binary_" BINARY_CERT "_end");

BackendConfigurer::BackendConfigurer(I_NVS &cfgnvs, I_NVS &eccnvs) noexcept
    : cfgnvs{&cfgnvs}, encnvs{&eccnvs} {}

backend_config_t BackendConfigurer::get() const {
    const auto len =
        (size_t)(cert_pem_end -
                 cert_pem_start); // NOLINT(clang-analyzer-security.PointerSub)
    return {.base_url = URL{cfgnvs->read(key::BACKEND_URL)},
            .auth_header{"Bearer " + encnvs->read(key::API_KEY)},
            .cert_pem = (const char *)cert_pem_start,
            .cert_len = len,
            .skip_cn_check = cfgnvs->read(key::SKIP_CN_CHECK) == B::yes};
}
