#ifndef WIFIMANAGER_HPP
#define WIFIMANAGER_HPP

#include "I_WiFi.hpp"
#include "Provisioning.hpp"
#include "Toggle.hpp"
#include "WiFiEventHandler.hpp"
#include "util.hpp"
#include <functional>
#include <memory>
#include <string>

class I_WiFiManager {
  public:
    virtual ~I_WiFiManager() noexcept {};
    virtual void init() = 0;
    virtual int connect() = 0;
    virtual int disconnect() = 0;
    virtual bool is_connected() = 0;
    virtual int reprovision() = 0;
};

template <typename T>
concept WiFiManagerTypes =
    util::I_EventGroup<typename T::EventGroup> &&
    WiFiHandle<typename T::WiFi> && I_Provisioning<typename T::Provisioning>;

template <WiFiManagerTypes T> class WiFiManager : public I_WiFiManager {
  protected:
    using WiFi = typename T::WiFi;
    using EGroup = typename T::EventGroup;
    using Handler = typename T::EventHandler;
    using Provisioning = typename T::Provisioning;

  public:
    template <typename HandlerFactory>
        requires util::Returns<HandlerFactory, Handler, EGroup &>
    WiFiManager(HandlerFactory handler_factory) noexcept;

    ~WiFiManager();
    void init() override;
    int connect() override;
    int disconnect() override;
    bool is_connected() override;
    int reprovision() override;

  protected:
    int connect_without_prov();
    int connect_with_prov();
    void switch_to_prov();
    void switch_to_no_prov();

    EGroup event_group;
    WiFi wifi_{};
    Toggle<Handler> handler;
    std::optional<Provisioning> provisioning{};
};

#include "WiFiManager.tpp"

#endif
