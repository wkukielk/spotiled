#ifndef CREDENTIALS_WIFI_MANAGER_HPP
#define CREDENTIALS_WIFI_MANAGER_HPP

#include "WiFiManager.hpp"
#include <string_view>

class CredentialsWiFiManager : public I_WiFiManager {
  public:
    CredentialsWiFiManager(std::string_view ssid, std::string_view passwd,
                           u_ptr<util::EventHandler> handler,
                           u_ptr<util::I_EventGroup> event_group);
    int init() override;
    int connect() override;
    int disconnect() override;
    bool is_connected() override;

  private:
    std::string ssid{};
    std::string passwd{};
    u_ptr<util::EventHandler> handler{};
    u_ptr<util::I_EventGroup> event_group{};
};

#endif
