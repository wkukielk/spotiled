#include "HTTPHandlerTestAux.hpp"
#include "errors.hpp"
#include "util.hpp"

SemaphoreHandle_t HTTPHandlerTestAux::sem{};
http::cbdata HTTPHandlerTestAux::cbdata{};

HTTPHandlerTestAux::HTTPHandlerTestAux(I_HTTPHandler &handler) {
    cbdata = handler.get_cbdata();
    auto status = esp_event_handler_instance_register(
        ESP_HTTP_CLIENT_EVENT, ESP_EVENT_ANY_ID, &handler_adapter, &cbdata,
        &handler_handle);
    if (status != ESP_OK)
        throw std::runtime_error{"failed to register"};
    sem = xSemaphoreCreateBinary();
    UTIL_THROW(!sem, InitError, "sem");
}

HTTPHandlerTestAux::~HTTPHandlerTestAux() noexcept {
    esp_event_handler_instance_unregister(ESP_HTTP_CLIENT_EVENT,
                                          ESP_EVENT_ANY_ID, &handler_handle);
    vSemaphoreDelete(sem);
}

void HTTPHandlerTestAux::post(esp_http_client_event &data) {
    static int counter = 0;
    cbdata.cnt = ++counter;
    esp_event_post(ESP_HTTP_CLIENT_EVENT, data.event_id, &data, sizeof(data),
                   portMAX_DELAY);
    xSemaphoreTake(sem, portMAX_DELAY);
}

void HTTPHandlerTestAux::handler_adapter(void *event_handler_arg,
                                         esp_event_base_t event_base,
                                         int32_t event_id, void *event_data) {
    static int curr_counter = 0;
    (void)event_handler_arg;
    (void)event_base;
    (void)event_id;
    if (cbdata.cnt != curr_counter) {
        curr_counter = cbdata.cnt;
        auto *http_data = static_cast<esp_http_client_event *>(event_data);
        http_data->user_data = &cbdata;
        (cbdata.callback)(http_data);
        xSemaphoreGive(sem);
    }
}