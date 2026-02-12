#ifndef M_DNS_CONFIG
#define M_DNS_CONFIG

#include "mDNS.hpp"
#include "nvs.hpp"
#include "util.hpp"

class mDNSConfig {
  public:
    explicit mDNSConfig(I_NVS &cfgnvs);
    u_ptr<I_mDNS> get() const;

  private:
    I_NVS *cfgnvs{};
};

#endif
