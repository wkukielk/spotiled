#include "nvs.hpp"

class Config {
  public:
    Config(I_NVS &nvs);

  private:
    I_NVS *nvs;
};
