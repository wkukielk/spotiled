#ifndef M_DNS_CONFIG
#define M_DNS_CONFIG

#include "I_mDNSFacade.hpp"
#include "mDNS.hpp"
#include "nvs.hpp"
#include "util.hpp"

class mDNSConfig {
  public:
    explicit mDNSConfig(I_NVS &cfgnvs, I_NVS &nvs);
    u_ptr<I_mDNSFacade> get() const;

  private:
    I_NVS *cfgnvs{};
    I_NVS *nvs{};
};

#endif
