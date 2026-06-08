#ifndef CREDENTIALS_WIFI_MANAGER_HPP
#define CREDENTIALS_WIFI_MANAGER_HPP

#include "WiFiManager.hpp"
#include <string_view>

template <typename T>
concept CredentialsWiFiManagerTypes =
    util::I_EventGroup<typename T::EventGroup> && I_WiFi<typename T::WiFi> &&
    std::is_constructible_v<typename T::EventHandler, typename T::EventGroup &,
                            uint8_t>;

template <CredentialsWiFiManagerTypes T>
class CredentialsWiFiManager : public I_WiFiManager {
    using EGroup = typename T::EventGroup;
    using Handler = typename T::EventHandler;
    using WiFi = typename T::WiFi;

  public:
    CredentialsWiFiManager(std::string_view ssid, std::string_view passwd);
    int init() override;
    int connect() override;
    int disconnect() override;
    bool is_connected() override;

  private:
    std::string ssid{};
    std::string passwd{};
    EGroup event_group{};
    Handler handler;
};

#include "CredentialsWiFiManager.tpp"

#endif
