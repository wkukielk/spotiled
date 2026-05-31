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

template <typename T_OTAClient> struct InnerConfig {
    T_OTAClient ota_client;
    u_ptr<I_WiFiManager> wifi_mgr;
};

class Runnable {
  public:
    virtual ~Runnable() noexcept {};
    virtual void run() = 0;
};

template <typename T>
class Runtime : public Runnable, public util::Notifiable<master::Data> {
    using EGroup = typename T::EGroup;
    using T_OTAThread = typename T::T_OTAThread;
    using T_OTAClient = typename T::T_OTAThread::Client;
    using T_ServerThread = typename T::T_ServerThread;

  public:
    static std::unique_ptr<Runnable> create(RuntimeConfig &&cfg);
    void run() override;
    void notify(u_ptr<master::Data> &&data) override { queue.send(move(data)); }

  private:
    Runtime(InnerConfig<T_OTAClient> &&cfg);
    esp_err_t setup();
    int perform();
    void cleanup();
    std::unique_ptr<master::Data> pop_queue();
    esp_err_t process(master::Data &data);

  protected:
    u_ptr<I_WiFiManager> wifi_mgr{};
    EGroup egroup{};
    T_OTAThread ota_thread;
    T_ServerThread server_thread;
    util::PointerQueue<master::Data> queue;
};

#include "runtime.tpp"

#endif
