#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include "I_WiFi.hpp"
#include "I_mDNS.hpp"
#include "nvs.hpp"
#include "time_aux.hpp"
#include <memory>
#include <optional>

class Global {
    Global();

  public:
    Global(Global &&) = default;
    ~Global() noexcept;

    void set_time(std::unique_ptr<I_Time> &&time);
    void set_nvs(std::unique_ptr<I_NVS> &&nvs);
    void set_mdns(std::unique_ptr<I_mDNS> &&mdns);
    I_Time &get_time() { return *time; }
    I_NVS &get_nvs() { return *nvs; }
    I_mDNS &get_mdns() { return *mdns; }

    static void init();
    static Global &get_global();
    static void deinit();

  private:
    static std::optional<Global> instance;
    std::unique_ptr<I_Time> time{};
    std::unique_ptr<I_NVS> nvs{};
    std::unique_ptr<I_mDNS> mdns{};
};

inline I_Time &time() { return Global::get_global().get_time(); }
inline I_NVS &nvs_() { return Global::get_global().get_nvs(); }
inline I_mDNS &mdns_() { return Global::get_global().get_mdns(); }

#endif
