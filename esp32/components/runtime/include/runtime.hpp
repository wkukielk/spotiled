#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include "OTAClient.hpp"
#include "URL.hpp"
#include "WiFiManager.hpp"

struct backend_config_t {
    URL base_url;
    std::string auth_header;
    const char *cert_pem;
    size_t cert_len;
    bool skip_cn_check;
};

struct RuntimeConfig {
    backend_config_t backend_cfg;
    u_ptr<I_WiFiManager> wifi_mgr;
    u_ptr<I_FullOTA> ota;
};

struct InnerConfig;

class Runtime {
  public:
    static std::unique_ptr<Runtime> create(RuntimeConfig &&cfg,
                                           u_ptr<util::I_EventGroup> &&egroup);
    void run();

  private:
    Runtime(InnerConfig &&cfg, u_ptr<util::I_EventGroup> &&egroup);
    void setup();

    static void update_fun(void *runtime);

    u_ptr<I_WiFiManager> wifi_mgr{};
    u_ptr<OTAClient> ota_client{};
    u_ptr<util::I_EventGroup> egroup{};
    u_ptr<util::Thread> ota_thread{};
};

#endif
