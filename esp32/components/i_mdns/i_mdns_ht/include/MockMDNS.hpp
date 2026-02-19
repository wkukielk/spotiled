#ifndef MOCK_MDNS_HPP
#define MOCK_MDNS_HPP

#include "I_mDNS.hpp"
#include <gmock/gmock.h>

class MockMDNS : public I_mDNS {
    using K = const char *;

  public:
    MOCK_METHOD(esp_err_t, set_hostname, (K hostname), (override));
    MOCK_METHOD(esp_err_t, subtype_add,
                (K instance, K service_type, K proto, K hostname, K subtype),
                (override));
    MOCK_METHOD(esp_err_t, set_instance_name, (K instance), (override));
    MOCK_METHOD(esp_err_t, add_service,
                (K instance, K service_type, K proto, uint16_t port,
                 mdns::txt_item_t txt[], size_t num_items),
                (override));
    MOCK_METHOD(esp_err_t, query_a,
                (K host_name, uint32_t timeout, esp_ip4_addr_t *addr),
                (override));
};

#endif
