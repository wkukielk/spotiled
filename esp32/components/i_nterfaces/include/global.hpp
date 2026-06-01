#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include "I_LED.hpp"
#include "I_WiFi.hpp"
#include "I_mDNS.hpp"
#include "nvs.hpp"
#include "time_aux.hpp"
#include <memory>

class Global {
    Global();
    ~Global() noexcept;

  public:
    void set_time(std::unique_ptr<I_Time> &&time);
    void set_nvs(std::unique_ptr<I_NVS> &&nvs);
    void set_wifi(std::unique_ptr<I_WiFi> &&wifi);
    void set_mdns(std::unique_ptr<I_mDNS> &&mdns);
    I_Time &get_time() { return *time; }
    I_NVS &get_nvs() { return *nvs; }
    I_WiFi &get_wifi() { return *wifi; }
    I_mDNS &get_mdns() { return *mdns; }

    static Global &get_global();
    static void deinit();
    static void init_default();

  private:
    static Global *instance;
    std::unique_ptr<I_Time> time{};
    std::unique_ptr<I_NVS> nvs{};
    std::unique_ptr<I_WiFi> wifi{};
    std::unique_ptr<I_mDNS> mdns{};
};

inline I_Time &time() { return Global::get_global().get_time(); }
inline I_NVS &nvs_() { return Global::get_global().get_nvs(); }
inline I_WiFi &wifi_() { return Global::get_global().get_wifi(); }
inline I_mDNS &mdns_() { return Global::get_global().get_mdns(); }

#endif
