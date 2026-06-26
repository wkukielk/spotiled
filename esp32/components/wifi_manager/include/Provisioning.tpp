#include "errors.hpp"
#include "esp_log.h"
#include "esp_wifi.h"
#include "global.hpp"
#include "straux.hpp"
#include "util.hpp"
#include "wifi_aux.hpp"
#include <stdexcept>

template <ProvisioningTypes T>
Provisioning<T>::Provisioning(EGroup &event_group, const char *prov_name)
    : event_group{&event_group}, handler{event_group, 3},
      service_name{prov_name} {
    wifi_prov.set_service_uuid(provisioning::custom_service_uuid);
}

template <ProvisioningTypes T> bool Provisioning<T>::is_provisioned() const {
    bool is_provisioned{};
    UTIL_THROW(wifi_prov.is_provisioned(&is_provisioned), WiFiManagerError,
               "is provisioned check");
    return is_provisioned;
}

template <ProvisioningTypes T> int Provisioning<T>::connect() {
    start();
    while (wifi::await_connection(*event_group) != ESP_OK) {
        UTIL_THROW(hot_reset(), WiFiManagerError, "hot reset ProvManager");
        time_().delay_ms(3000); // wait for message to user,
                                // can't synchronise any other way
        wifi_prov.stop_provisioning();
        event_group->wait(wifi::end_bit);
        start();
    }
    return ESP_OK;
}

template <ProvisioningTypes T> void Provisioning<T>::start() {
    auto status = wifi_prov.start_provisioning(security, &pop,
                                               service_name.c_str(), nullptr);
    UTIL_THROW(status, InitError, "Failed to start provisioning");
}

template <ProvisioningTypes T> int Provisioning<T>::hot_reset() {
    return wifi_prov.reset_sm_state_on_failure();
}

template <ProvisioningTypes T> int Provisioning<T>::reset() {
    return wifi_prov.reset_provisioning();
}
