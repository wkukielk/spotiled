#include "WiFiManager.hpp"
#include "nvs.hpp"

struct Config {
    u_ptr<I_WiFiManager> wifi_mgr;
};

class ConfigReader {
  public:
    ConfigReader(I_NVS &cfgnvs);
    void preconfigure() const;
    Config get_config() const;

  private:
    Provisioning *get_provisioning(util::EventGroup &event_group) const;

  private:
    I_NVS *cfgnvs;
};
