#ifndef CREDENTIALS_WIFI_MANAGER_HPP
#define CREDENTIALS_WIFI_MANAGER_HPP

#include "WiFiManager.hpp"
#include <string_view>


template<typename Types, typename T>
concept HandlerFactory = WiFiManagerTypes<Types> && util::Returns<T, typename Types::EventHandler, typename Types::EventGroup&>;

template <WiFiManagerTypes T>
class CredentialsWiFiManager : public I_WiFiManager {
  using EGroup = typename T::EventGroup;
  using Handler = typename T::EventHandler;
  using WiFi = typename T::WiFi;

  public:
    template <typename Factory>
    requires HandlerFactory<T, Factory>
    CredentialsWiFiManager(std::string_view ssid, std::string_view passwd,
                           Factory factory);
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
