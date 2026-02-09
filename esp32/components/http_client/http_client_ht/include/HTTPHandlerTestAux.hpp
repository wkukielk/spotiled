#ifndef HTTP_HANDLER_TEST_AUX_HPP
#define HTTP_HANDLER_TEST_AUX_HPP

#include "HTTPHandler.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "freertos/FreeRTOS.h"
#pragma GCC diagnostic pop

class HTTPHandlerTestAux {
  public:
    HTTPHandlerTestAux(I_HTTPHandler &handler);
    ~HTTPHandlerTestAux() noexcept;
    void post(esp_http_client_event &data);
    static void handler_adapter(void *event_handler_arg,
                                esp_event_base_t event_base, int32_t event_id,
                                void *event_data);
    esp_event_handler_instance_t handler_handle{};
    static http::cbdata cbdata;
    static SemaphoreHandle_t sem;
};

#endif
