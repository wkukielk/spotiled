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

class WiFiManager {
  public:
    WiFiManager(std::unique_ptr<util::I_EventGroup> &&event_group,
                std::unique_ptr<I_ToggleEventHandler> &&handler,
                std::unique_ptr<I_Provisioning> &&provisioning);
    ~WiFiManager();
    int init();
    int connect();
    int disconnect();
    bool is_connected();

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
