#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include "I_WiFi.hpp"
#include "I_mDNSFacade.hpp"
#include "errors.hpp"
#include "nvs.hpp"
#include "time_aux.hpp"
#include <memory>
#include <optional>

class Global {
  public:
    static void set_time(std::unique_ptr<I_Time> &&time);
    static void set_nvs(std::unique_ptr<I_NVS> &&nvs);
    static void set_mdns(std::unique_ptr<I_mDNSFacade> &&mdns);
    static I_Time &get_time() { return *time; }
    static I_NVS &get_nvs() { return *nvs; }
    static I_mDNSFacade &get_mdns() { return *mdns; }

  private:
    static std::unique_ptr<I_Time> time;
    static std::unique_ptr<I_NVS> nvs;
    static std::unique_ptr<I_mDNSFacade> mdns;
};

inline I_Time &time_() { return Global::get_time(); }
inline I_NVS &nvs_() { return Global::get_nvs(); }
inline I_mDNSFacade &mdns_() { return Global::get_mdns(); }

class Netif {
  public:
    Netif();
    Netif(Netif &&) = default;
    ~Netif();
};

template <typename T> class GlobalHandle : public util::MoveSafe<T> {
    using Fun = std::function<void(std::unique_ptr<T> &&)>;

  public:
    GlobalHandle(Fun setter, std::unique_ptr<T> instance) {
        if (GlobalHandle::setter.has_value())
            throw InitError(ESP_FAIL, "GlobalHandle already initialized");
        GlobalHandle::setter = std::move(setter);
        GlobalHandle::setter.value()(std::move(instance));
    }
    GlobalHandle(GlobalHandle &&) = default;
    GlobalHandle(GlobalHandle &) = delete;
    ~GlobalHandle() noexcept {
        if (this->last()) {
            GlobalHandle::setter.value()(nullptr);
            GlobalHandle::setter.reset();
        }
    }

  private:
    static std::optional<Fun> setter;
};

template <typename T>
std::optional<std::function<void(std::unique_ptr<T> &&)>>
    GlobalHandle<T>::setter{};

class TimeHandle {
  public:
    TimeHandle(std::unique_ptr<I_Time> time)
        : time{Global::set_time, std::move(time)} {}

  private:
    GlobalHandle<I_Time> time;
};

class NVSHandle {
  public:
    NVSHandle(std::unique_ptr<I_NVS> nvs)
        : nvs{Global::set_nvs, std::move(nvs)} {}

  private:
    GlobalHandle<I_NVS> nvs;
};

class MDNSHandle {
  public:
    MDNSHandle(std::unique_ptr<I_mDNSFacade> mdns)
        : mdns{Global::set_mdns, std::move(mdns)} {}

  private:
    GlobalHandle<I_mDNSFacade> mdns;
};

#endif
