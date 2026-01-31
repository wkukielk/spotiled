#ifndef WIFIMANAGER_HPP
#define WIFIMANAGER_HPP

#include "I_WiFi.hpp"
#include "Provisioning.hpp"
#include "ToggleEventHandler.hpp"
#include "WiFiEventHandler.hpp"
#include "util.hpp"
#include <functional>
#include <memory>
#include <string>

class I_WiFiManager {
  public:
    virtual ~I_WiFiManager() noexcept {};
    virtual int init() = 0;
    virtual int connect() = 0;
    virtual int disconnect() = 0;
    virtual bool is_connected() = 0;
};

class WiFiManager : public I_WiFiManager {
  public:
    WiFiManager(std::unique_ptr<util::I_EventGroup> &&event_group,
                std::unique_ptr<I_ToggleEventHandler> &&handler,
                std::unique_ptr<I_Provisioning> &&provisioning);
    ~WiFiManager();
    int init() override;
    int connect() override;
    int disconnect() override;
    bool is_connected() override;

  private:
    int connect_without_prov();
    int connect_with_prov();
    void switch_to_prov();
    void switch_to_no_prov();
    std::unique_ptr<util::I_EventGroup> event_group;
    std::unique_ptr<I_ToggleEventHandler> handler;
    std::unique_ptr<I_Provisioning> provisioning;
};

#endif
