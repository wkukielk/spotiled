#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include "I_WiFi.hpp"
#include "I_mDNS.hpp"
#include "nvs.hpp"
#include "time_aux.hpp"
#include <memory>

class Netif {
  public:
    Netif();
    ~Netif() noexcept;
};

class DefaultEventLoop {
  public:
    DefaultEventLoop();
    ~DefaultEventLoop() noexcept;
};

class Global {
  private:
    Global();

  public:
    void set_time(std::unique_ptr<I_Time> &&time);
    void set_nvs(std::unique_ptr<I_NVS> &&nvs);
    void set_wifi(std::unique_ptr<I_WiFi> &&wifi);
    void set_mdns(std::unique_ptr<I_mDNS> &&mdns);
    inline I_Time &get_time() { return *time; }
    inline I_NVS &get_nvs() { return *nvs; }
    inline I_WiFi &get_wifi() { return *wifi; }
    inline I_mDNS &get_mdns() { return *mdns; }

    static Global *get_global();
    static void deinit();
    static void init_default();

  private:
    static Global *instance;
    std::unique_ptr<DefaultEventLoop> eloop{};
    std::unique_ptr<Netif> netif{};
    std::unique_ptr<I_Time> time{};
    std::unique_ptr<I_NVS> nvs{};
    std::unique_ptr<I_WiFi> wifi{};
    std::unique_ptr<I_mDNS> mdns{};
};

#define time() Global::get_global()->get_time()
#define nvs() Global::get_global()->get_nvs()
#define wifi() Global::get_global()->get_wifi()
#define mdns() Global::get_global()->get_mdns()

#endif
