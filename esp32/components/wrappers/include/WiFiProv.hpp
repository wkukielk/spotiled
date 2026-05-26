#ifndef WIFI_PROV_HPP
#define WIFI_PROV_HPP

#include "I_WiFiProv.hpp"
#include "network_provisioning/manager.h"
#include "network_provisioning/scheme_ble.h"

namespace wifi_prov {
extern network_prov_mgr_config_t cfg;
};

class WiFiProv : public I_WiFiProv {
  public:
    WiFiProv(network_prov_mgr_config_t *config);
    ~WiFiProv() noexcept;
    esp_err_t is_provisioned(bool *provisioned) override;
    esp_err_t start_provisioning(WiFiProvSecurity security,
                                 const void *network_prov_sec_params,
                                 const char *service_name,
                                 const char *service_key) override;
    void stop_provisioning() override;
    esp_err_t reset_sm_state_on_failure() override;
    esp_err_t reset_provisioning() override;
    esp_err_t set_service_uuid(uint8_t *uuid128) override;
};

#endif
