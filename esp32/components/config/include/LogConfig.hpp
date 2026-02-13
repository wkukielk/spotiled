#ifndef LOG_CONFIG_HPP
#define LOG_CONFIG_HPP

#include "nvs.hpp"

class LogConfig {
  public:
    explicit LogConfig(I_NVS &cfgnvs);
    void configure() const noexcept;

  private:
    I_NVS *cfgnvs{};
};

#endif
