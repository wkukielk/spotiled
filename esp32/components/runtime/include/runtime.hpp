#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include "OTAClient.hpp"
#include "URL.hpp"
#include "WiFiManager.hpp"
#include "light.hpp"
#include "master/runtime_helpers.hpp"
#include "queue.hpp"
#include "runtime_data.hpp"
#include <chrono>

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

template <typename T> class Runtime : public util::Notifiable<master::Data> {
    using EGroup = typename T::EGroup;
    using T_OTAThread = typename T::T_OTAThread;
    using T_OTAClient = typename T::T_OTAThread::Client;
    using T_ServerThread = typename T::T_ServerThread;
    using T_UDPThread = typename T::T_UDPThread;
    using T_LEDC = typename T::T_LEDC;
    using T_Light = typename T::T_Light;
    using T_ServerHandler = typename T_ServerThread::Handler;
    using T_UDPHandler = typename T_UDPThread::Handler;

  public:
    virtual ~Runtime() noexcept {};
    static u_ptr<Runtime> create(RuntimeConfig &&cfg);
    void run();
    void notify(u_ptr<master::Data> &&data) override { queue.send(move(data)); }

  private:
    Runtime(RuntimeConfig &&cfg);
    template <typename F> esp_err_t guarded(const char *step, F &&f) noexcept;
    void handle_connect() noexcept;
    esp_err_t connect();
    void handle_start() noexcept;
    esp_err_t start();
    std::expected<T_OTAClient, esp_err_t> make_ota_client();
    int perform();
    void cleanup();
    std::unique_ptr<master::Data> pop_queue();
    esp_err_t process(master::Data &data);

  protected:
    static constexpr gpio_num_t status_light_pin = GPIO_NUM_4;
    static constexpr light::pattern wifi_error_blink{std::chrono::seconds{3},
                                                     std::chrono::seconds{1}};
    static constexpr light::pattern provisioning_blink{std::chrono::seconds{1},
                                                       std::chrono::seconds{1}};
    static constexpr light::pattern error_blink{std::chrono::seconds{1},
                                                std::chrono::seconds{3}};

    backend_config_t backend_cfg;
    u_ptr<I_WiFiManager> wifi_mgr{};
    T_Light status_light;
    EGroup egroup{};
    // Needs the resolved backend URL, so it's created in setup().
    std::optional<T_OTAThread> ota_thread{};
    T_LEDC ledc{};
    T_ServerThread server_thread;
    T_UDPThread udp_thread;
    util::PointerQueue<master::Data> queue;
};

#include "runtime.tpp"

#endif
