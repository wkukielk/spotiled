#ifndef I_MDNS_FACADE_HPP
#define I_MDNS_FACADE_HPP

#include "esp_err.h"
#include "esp_netif.h"

// The subset of mDNS operations used after initial setup: renaming the
// instance and resolving hostnames. Setup-only operations (hostname,
// subtypes, service registration) live on I_mDNS instead.
class I_mDNSFacade {
    using K = const char *;

  public:
    virtual ~I_mDNSFacade() noexcept {};
    virtual esp_err_t set_instance_name(K instance) = 0;
    virtual esp_err_t query_a(K host_name, uint32_t timeout,
                              esp_ip4_addr_t *addr) = 0;
};

#endif
