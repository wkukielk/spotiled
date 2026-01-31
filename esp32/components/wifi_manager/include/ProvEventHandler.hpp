#ifndef PROV_EVENT_HANDLER_HPP
#define PROV_EVENT_HANDLER_HPP

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "esp_event.h"
#pragma GCC diagnostic pop

#include "I_WiFiProv.hpp"
#include "util.hpp"
#include <memory>

// only one per program
class ProvEventHandler : public util::EventHandler {
  public:
    ProvEventHandler(util::I_EventGroup &event_group, uint8_t retries,
                     util::Register reg = util::Register::YES);
    ~ProvEventHandler();
    esp_err_t register_self();
    std::span<const util::cbdata> get_cbdata() const noexcept override;
    ProvReason get_reason() const noexcept;

    static void handler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data);
    static void disconnect_handler(void *arg, esp_event_base_t event_base,
                                   int32_t event_id, void *event_data);
    static util::I_EventGroup *event_group;
    static uint8_t retries;
    static int8_t retries_left;
    static esp_event_handler_instance_t handle;
    static esp_event_handler_instance_t disconnect_handle;
    static esp_event_handler_instance_t ip_handle;
    static const util::cbdata cbdata[2];
    static ProvReason reason;
};

#endif
