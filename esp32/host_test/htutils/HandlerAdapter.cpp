#include "HandlerAdapter.hpp"
#include "util.hpp"
#include <algorithm>

SemaphoreHandle_t HandlerAdapter::sem{};
std::vector<util::cbdata> HandlerAdapter::cbdata{};

HandlerAdapter::HandlerAdapter(util::EventHandler &handler) {
    auto cbd = handler.get_cbdata();
    cbdata = std::vector<util::cbdata>{cbd.data(), cbd.data() + cbd.size()};
    for (auto &cbd : cbdata) {
        auto instance = register_handler(cbd);
        UTIL_CHECK(instance == nullptr, "failed to register handler");
        cbd.instance = instance;
    }
    sem = xSemaphoreCreateBinary();
    UTIL_CHECK(sem == NULL, "sem is NULL");
}

HandlerAdapter::~HandlerAdapter() noexcept {
    for (const auto &cbd : cbdata)
        esp_event_handler_instance_unregister(cbd.base, cbd.id, cbd.instance);
    vSemaphoreDelete(sem);
}

esp_event_handler_instance_t
HandlerAdapter::register_handler(const util::cbdata &cbdata) {
    esp_event_handler_instance_t handle{};
    esp_event_handler_instance_register(
        cbdata.base, cbdata.id, &handler_adapter, (void *)&cbdata, &handle);
    return handle;
}

void HandlerAdapter::post(void *data, size_t size, esp_event_base_t base,
                          int32_t id) {
    static int counter = 0;
    auto cbd = std::find_if(cbdata.begin(), cbdata.end(), [=](const auto &cbd) {
        if (cbd.base == ESP_EVENT_ANY_BASE)
            return true;
        return cbd.base == base && (cbd.id == ESP_EVENT_ANY_ID || cbd.id == id);
    });
    UTIL_CHECK(cbd == cbdata.end(), "unregistered base or id");
    cbd->cnt = ++counter;
    UTIL_CHECK(esp_event_post(base, id, data, size, portMAX_DELAY),
               "failed to post");
    xSemaphoreTake(sem, portMAX_DELAY);
}

void HandlerAdapter::handler_adapter(void *event_handler_arg,
                                     esp_event_base_t event_base,
                                     int32_t event_id, void *event_data) {
    static int curr_counter = 0;
    auto cbd = static_cast<util::cbdata *>(event_handler_arg);
    if (cbd->cnt != curr_counter) {
        curr_counter = cbd->cnt;
        util::HandlerProxy::callback(event_handler_arg, event_base, event_id,
                                     event_data);
        xSemaphoreGive(sem);
    }
}