#include "WiFiManager.hpp"
#include "WiFiManagerFactory.hpp"
#include "nvs.hpp"

struct Config {
    u_ptr<I_WiFiManager> wifi_mgr;
};

class ConfigReader {
  public:
    explicit ConfigReader(I_NVS &cfgnvs);
    void preconfigure() const;
    Config get_config() const;

  private:
    I_NVS *cfgnvs;
    WiFiManagerFactory wifi_mgr_factory;
};
