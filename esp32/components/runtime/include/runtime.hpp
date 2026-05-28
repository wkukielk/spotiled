#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include "OTAClient.hpp"
#include "URL.hpp"
#include "WiFiManager.hpp"
#include "master/runtime_helpers.hpp"
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
};

struct InnerConfig {
    OTAClient ota_client;
    u_ptr<I_WiFiManager> wifi_mgr;
};

class Runnable {
  public:
    virtual ~Runnable() noexcept {};
    virtual void run() = 0;
};

template <typename EGroup>
class Runtime : public Runnable, public util::Notifiable<master::Data> {
  public:
    template <typename T_FullOTA>
    static std::unique_ptr<Runnable> create(RuntimeConfig &&cfg);
    void run() override;
    void notify(u_ptr<master::Data> &&data) override { queue.send(move(data)); }

  protected:
    Runtime(InnerConfig &&cfg);
    void setup();
    int perform();
    void cleanup();
    std::unique_ptr<master::Data> pop_queue();
    esp_err_t process(master::Data &data);

    u_ptr<I_WiFiManager> wifi_mgr{};
    EGroup egroup{};
    OTAThread ota_thread;
    util::PointerQueue<master::Data> queue;
};

#include "runtime.tpp"

#endif
