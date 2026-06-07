#ifndef PROVISIONING_MANAGER_HPP
#define PROVISIONING_MANAGER_HPP

#include "I_WiFiProv.hpp"
#include "safe_unique_ptr.hpp"
#include "time_aux.hpp"
#include "util.hpp"
#include "wifi_aux.hpp"
#include <functional>
#include <memory>

struct ProvEssentials {
    wifi::ProvGetter get_wifi_prov;
    wifi::HandlerGetter get_handler;
};

class I_Provisioning : util::Toggle {
  public:
    virtual ~I_Provisioning() noexcept {};
    virtual bool is_provisioned() = 0;
    virtual int connect() = 0;
    virtual I_WiFiProv &get_wifi_prov() = 0;
    using util::Toggle::is_on;
    using util::Toggle::off;
    using util::Toggle::on;
};

class Provisioning : public I_Provisioning {
  public:
    Provisioning(const ProvEssentials &essentials,
                 util::EventGroupBase &event_group, const char *prov_name);
    esp_err_t on() override;
    esp_err_t off() override;
    bool is_on() const override;
    bool is_provisioned() override;
    int connect() override;
    I_WiFiProv &get_wifi_prov() override;

  private:
    void start();
    int hot_reset();
    int reset();

    ProvEssentials essentials;
    safe_unique_ptr<I_WiFiProv> wifi_prov{};
    safe_unique_ptr<util::EventHandler> handler{};
    util::EventGroupBase *event_group;
    std::string service_name;
    const WiFiProvSecurity security = WiFiProvSecurity::S1;
    const char pop{};
    static uint8_t custom_service_uuid[16];
};

namespace provisioning {
void print_qr(const char *name, const char *transport);
int get_service_name(char *service_name, size_t len);
} // namespace provisioning

#endif
