#ifndef PROV_EVENT_HANDLER_HPP
#define PROV_EVENT_HANDLER_HPP

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "esp_event.h"
#pragma GCC diagnostic pop

#include "I_DftEventLoop.hpp"
#include "I_WiFiProv.hpp"
#include "util.hpp"
#include <memory>

// only one per program
template <util::I_EventGroup EGroup, I_DftEventLoop ELoop>
class ProvEventHandler {
  public:
    ProvEventHandler(EGroup &event_group, uint8_t retries);
    ~ProvEventHandler();
    esp_err_t register_self();
    ProvReason get_reason() const noexcept;

    static void handler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data);
    static void disconnect_handler(void *arg, esp_event_base_t event_base,
                                   int32_t event_id, void *event_data);
    static EGroup *event_group;
    static uint8_t retries;
    static int8_t retries_left;
    static esp_event_handler_instance_t handle;
    static esp_event_handler_instance_t disconnect_handle;
    static ProvReason reason;
};

extern const char *PEH_TAG;

#include "ProvEventHandler.tpp"

#endif
