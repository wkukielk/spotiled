#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include "OTAClient.hpp"
#include "URL.hpp"
#include "WiFiManager.hpp"
#include "queue.hpp"
#include "runtime_data.hpp"

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

class Runnable {
    virtual void run() = 0;
};

class Runtime : public Runnable, public util::Notifiable<master::Data> {
  public:
    static std::unique_ptr<Runtime> create(RuntimeConfig &&cfg,
                                           u_ptr<util::I_EventGroup> &&egroup);
    void run();
    void notify(u_ptr<master::Data> &&data) override;

  private:
    Runtime(InnerConfig &&cfg, u_ptr<util::I_EventGroup> &&egroup);
    void setup();
    int loop();
    void cleanup();
    std::unique_ptr<master::Data> pop_queue();
    esp_err_t perform(master::Data &data);

    static void update_fun(void *runtime);

    u_ptr<I_WiFiManager> wifi_mgr{};
    u_ptr<OTAClient> ota_client{};
    u_ptr<util::I_EventGroup> egroup{};
    u_ptr<util::Thread> ota_thread{};
    util::PointerQueue<master::Data> queue;
};

#endif
