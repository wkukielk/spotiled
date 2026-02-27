#ifndef BACKEND_CONFIG_HPP
#define BACKEND_CONFIG_HPP

#include "nvs.hpp"
#include "runtime.hpp"

class BackendConfigurer {
  public:
    explicit BackendConfigurer(I_NVS &cfgnvs, I_NVS &ecnnvs) noexcept;
    backend_config_t get() const;

  private:
    I_NVS *cfgnvs{};
    I_NVS *encnvs{};
};

#endif
