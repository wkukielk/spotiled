#include "LogConfig.hpp"
#include "WiFiManager.hpp"
#include "WiFiManagerFactory.hpp"
#include "mDNSConfig.hpp"
#include "nvs.hpp"

struct Config {
    u_ptr<I_WiFiManager> wifi_mgr;
    u_ptr<I_mDNS> mdns;
};

class Configurer {
  public:
    explicit Configurer(I_NVS &cfgnvs);
    void preconfigure() const;
    Config get_config() const;

  private:
    I_NVS *cfgnvs;
    WiFiManagerFactory wifi_mgr_factory;
    mDNSConfig mdns_configurer;
    LogConfig log_configurer;
};
