#include "mDNS.hpp"
#include "errors.hpp"
#include "mdns.h"
#include "util.hpp"
#include <ranges>

using namespace std::ranges;
using namespace std::views;
using namespace std;

namespace {
mdns_txt_item_t to_txt_item(mdns::txt_item_t item) {
    return mdns_txt_item_t{item.key, item.value};
}
} // namespace

mDNS::mDNS() { UTIL_THROW(mdns_init(), InitError, "mdns"); }
esp_err_t mDNS::set_hostname(K hostname) { return mdns_hostname_set(hostname); }
esp_err_t mDNS::subtype_add(K instance, K service_type, K proto, K hostname,
                            K subtype) {
    return mdns_service_subtype_add_for_host(instance, service_type, proto,
                                             hostname, subtype);
}
esp_err_t mDNS::set_instance_name(K instance) {
    return mdns_instance_name_set(instance);
}
esp_err_t mDNS::add_service(K instance, K service_type, K proto, uint16_t port,
                            span<mdns::txt_item_t> items) {
    auto txt_items = items | views::transform(to_txt_item) | to<vector>();
    return mdns_service_add(instance, service_type, proto, port,
                            txt_items.data(), txt_items.size());
}
esp_err_t mDNS::query_a(K host_name, uint32_t timeout, esp_ip4_addr_t *addr) {
    return mdns_query_a(host_name, timeout, addr);
}
