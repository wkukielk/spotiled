#include "Provisioning.hpp"
#include "I_WiFiProv.hpp"
#include "errors.hpp"
#include "esp_log.h"
#include "esp_wifi.h"
#include "global.hpp"
#include "straux.hpp"
#include "time_aux.hpp"
#include "util.hpp"
#include "wifi_aux.hpp"
#include <stdexcept>

#define QRCODE_BASE_URL "https://espressif.github.io/esp-jumpstart/qrcode.html"
static const char *TAG = "Provisioning";

uint8_t Provisioning::custom_service_uuid[16] = {
    /* little-endian */
    0xb4, 0xdf, 0x5a, 0x1c, 0x3f, 0x6b, 0xf4, 0xbf,
    0xea, 0x4a, 0x82, 0x03, 0x04, 0x90, 0x1a, 0x02,
};

Provisioning::Provisioning(const ProvEssentials &essentials,
                           util::EventGroupBase &event_group,
                           const char *prov_name)
    : essentials{essentials}, event_group{&event_group},
      service_name{prov_name} {}

esp_err_t Provisioning::on() {
    wifi_prov.reset(essentials.get_wifi_prov());
    wifi_prov->set_service_uuid(custom_service_uuid);
    handler.reset(essentials.get_handler(*event_group));
    return ESP_OK;
}

esp_err_t Provisioning::off() {
    handler.reset();
    wifi_prov.reset();
    return ESP_OK;
}

bool Provisioning::is_on() const { return wifi_prov; }

I_WiFiProv &Provisioning::get_wifi_prov() { return *wifi_prov; }

bool Provisioning::is_provisioned() {
    bool is_provisioned{};
    UTIL_THROW(wifi_prov->is_provisioned(&is_provisioned), WiFiManagerError,
               "is provisioned check");
    return is_provisioned;
}

int Provisioning::connect() {
    start();
    while (wifi::await_connection(*event_group) != ESP_OK) {
        UTIL_THROW(hot_reset(), WiFiManagerError, "hot reset ProvManager");
        time().delay_ms(3000); // wait for message to user,
                               // can't synchronise any other way
        wifi_prov->stop_provisioning();
        event_group->wait(wifi::end_bit);
        start();
    }
    return ESP_OK;
}

void Provisioning::start() {
    auto status = wifi_prov->start_provisioning(security, &pop,
                                                service_name.c_str(), nullptr);
    UTIL_THROW(status, InitError, "Failed to start provisioning");
}

int Provisioning::hot_reset() { return wifi_prov->reset_sm_state_on_failure(); }

int Provisioning::reset() { return wifi_prov->reset_provisioning(); }

namespace provisioning {
void wifi_print_qr(const char *name, const char *transport) {
    if (!name || !transport) {
        ESP_LOGW(TAG, "Cannot generate QR code payload. Data missing.");
        return;
    }
    char payload[150] = {0};
    snprintf(payload, sizeof(payload),
             "{\"ver\":\"%s\",\"name\":\"%s\""
             ",\"transport\":\"%s\"}",
             "v1", name, transport);
    ESP_LOGI(TAG, "Copy paste the below URL in a browser.\n%s?data=%s",
             QRCODE_BASE_URL, payload);
}

int get_service_name(char *service_name, size_t len) {
    uint8_t eth_mac[6];
    const char *ssid_prefix = "PROV_";
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
    return snprintf(service_name, len, "%s%02X%02X%02X", ssid_prefix,
                    eth_mac[3], eth_mac[4], eth_mac[5]);
}
} // namespace provisioning