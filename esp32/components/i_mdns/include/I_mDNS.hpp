#ifndef I_MDNS_HPP
#define I_MDNS_HPP

#include "esp_err.h"
#include "esp_netif.h"

namespace mdns {
struct txt_item_t {
    const char *key;
    const char *value;
};
} // namespace mdns

class I_mDNS {
    using K = const char *;

  public:
    virtual esp_err_t set_hostname(K hostname) = 0;
    virtual esp_err_t subtype_add(K instance, K service_type, K proto,
                                  K hostname, K subtype) = 0;
    virtual esp_err_t instance_name_set(K instance) = 0;
    virtual esp_err_t service_add(K instance, K service_type, K proto,
                                  uint16_t port, mdns::txt_item_t txt[],
                                  size_t num_items) = 0;
    virtual esp_err_t mdns_query_a(K host_name, uint32_t timeout,
                                   esp_ip4_addr_t *addr) = 0;
};

#endif
