#ifndef WIFI_EVENT_HANDLER_HPP
#define WIFI_EVENT_HANDLER_HPP

#include "I_WiFi.hpp"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "util.hpp"

// only one per program
class WiFiEventHandler : public util::EventHandler {
  public:
    WiFiEventHandler(util::I_EventGroup &event_group, uint8_t retries,
                     util::Register reg = util::Register::YES);
    ~WiFiEventHandler();
    esp_err_t register_self();
    std::span<const util::cbdata> get_cbdata() const noexcept override;

  private:
    static void handler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data);
    static void ip_handler(void *arg, esp_event_base_t event_base,
                           int32_t event_id, void *event_data);

    static util::I_EventGroup *event_group;
    static uint8_t max_retries;
    static int8_t retries_left;
    static std::vector<util::cbdata> cbdata;
};

#endif
