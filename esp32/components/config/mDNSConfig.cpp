#include "mDNSConfig.hpp"

mDNSConfig::mDNSConfig(I_NVS &cfgnvs) : cfgnvs{&cfgnvs} {}

u_ptr<I_mDNS> mDNSConfig::get() const { return mkuptr<mDNS>(); }
