#ifndef BACKEND_CONFIG_HPP
#define BACKEND_CONFIG_HPP

#include "nvs.hpp"
#include "runtime.hpp"

class BackendConfigurer {
  public:
    explicit BackendConfigurer(I_NVS &cfgnvs) noexcept;
    backend_config_t get() const;

  private:
    I_NVS *cfgnvs{};
};

#endif
