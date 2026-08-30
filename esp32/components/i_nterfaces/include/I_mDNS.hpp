#ifndef I_MDNS_HPP
#define I_MDNS_HPP

#include "I_mDNSFacade.hpp"
#include <span>

namespace mdns {
struct txt_item_t {
    const char *key;
    const char *value;
};
} // namespace mdns

// Setup-only mDNS operations, used once at boot to configure the service.
// set_instance_name/query_a (the operations still used at runtime) live on
// the smaller I_mDNSFacade base instead.
class I_mDNS : public I_mDNSFacade {
    using K = const char *;

  public:
    virtual esp_err_t set_hostname(K hostname) = 0;
    virtual esp_err_t subtype_add(K instance, K service_type, K proto,
                                  K hostname, K subtype) = 0;
    virtual esp_err_t add_service(K instance, K service_type, K proto,
                                  uint16_t port,
                                  std::span<mdns::txt_item_t> items) = 0;
};

#endif
