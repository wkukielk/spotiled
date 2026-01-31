#ifndef HANDLER_ADAPTER_HPP
#define HANDLER_ADAPTER_HPP

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "freertos/FreeRTOS.h"
#pragma GCC diagnostic pop

#include "util.hpp"
#include <vector>

class HandlerAdapter {
  public:
    HandlerAdapter(util::EventHandler &handler);
    ~HandlerAdapter() noexcept;
    esp_event_handler_instance_t register_handler(const util::cbdata &cbdata);
    void post(void *data, size_t size, esp_event_base_t base, int32_t id);
    static void handler_adapter(void *event_handler_arg,
                                esp_event_base_t event_base, int32_t event_id,
                                void *event_data);
    static std::vector<util::cbdata> cbdata;
    static SemaphoreHandle_t sem;
};

#endif
