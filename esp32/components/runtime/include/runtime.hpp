#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include "OTAClient.hpp"
#include "URL.hpp"
#include "WiFiManager.hpp"
#include "queue.hpp"
#include "states.hpp"

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

namespace runtime {
class StateFactory {
    using State = sm::State<runtime::StateData, runtime::Data>;

  public:
    StateFactory();
    std::unique_ptr<State> get(runtime::StateData &&data) const;
};
} // namespace runtime

class Runtime : public Runnable, util::Notifiable<runtime::Data> {
    using Data = runtime::Data;
    using StateData = runtime::StateData;

  public:
    static std::unique_ptr<Runtime> create(RuntimeConfig &&cfg,
                                           u_ptr<util::I_EventGroup> &&egroup);
    void run();
    void notify(u_ptr<Data> &&data) override;

  private:
    Runtime(InnerConfig &&cfg, u_ptr<util::I_EventGroup> &&egroup);
    void setup();
    int loop();
    void cleanup();

    std::unique_ptr<Data> pop_queue();
    void change_state(StateData &&next_state_data);

    static void update_fun(void *runtime);

    u_ptr<I_WiFiManager> wifi_mgr{};
    u_ptr<OTAClient> ota_client{};
    u_ptr<util::I_EventGroup> egroup{};
    u_ptr<util::Thread> ota_thread{};
    util::PointerQueue<Data> queue;
    std::unique_ptr<sm::State<StateData, Data>> state{};
    runtime::StateFactory state_factory;
};

#endif
