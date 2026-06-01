#include "MockDftEventLoop.hpp"
#include "errors.hpp"
#include <algorithm>
#include <atomic>

using namespace std;

static void proxy_callback(cbdata &cbd, esp_event_base_t base, int32_t id,
                           void *data) {
    (*cbd.handler)(cbd.handler_arg, base, id, data);
}

std::unique_ptr<MockDftEventLoop> MockDftEventLoop::instance{};

static bool is_matched(cbdata &cbd, esp_event_base_t base, int32_t id) {
    if (cbd.base == ESP_EVENT_ANY_BASE)
        return true;
    return cbd.base == base && (cbd.id == ESP_EVENT_ANY_ID || cbd.id == id);
}

esp_err_t MockDftEventLoop::handler_register(
    esp_event_base_t event_base, int32_t event_id,
    esp_event_handler_t event_handler, void *event_handler_arg,
    esp_event_handler_instance_t *instance) noexcept {
    auto &cbd =
        handlers_data.emplace_back(new cbdata{.base = event_base,
                                              .id = event_id,
                                              .handler = event_handler,
                                              .handler_arg = event_handler_arg,
                                              .instance{},
                                              .eloop = *this,
                                              .cnt = 0});
    auto err = DftEventLoop::get().handler_register(
        event_base, event_id, MockDftEventLoop::handler_adapter, cbd.get(),
        instance);
    if (err != ESP_OK)
        handlers_data.pop_back();
    cbd->instance = *instance;
    return err;
}
esp_err_t MockDftEventLoop::handler_unregister(
    esp_event_base_t event_base, int32_t event_id,
    esp_event_handler_instance_t instance) noexcept {

    auto p = ranges::find_if(
        handlers_data, [=](auto &cbd) { return cbd->instance == instance; });
    assert(p != handlers_data.end());
    UTIL_RETURN(
        DftEventLoop::get().handler_unregister(event_base, event_id, instance));
    erase(handlers_data, *p);
    return ESP_OK;
}

esp_err_t MockDftEventLoop::post(esp_event_base_t event_base, int32_t event_id,
                                 const void *event_data, size_t event_data_size,
                                 TickType_t ticks_to_wait) noexcept {
    static int counter = 0;
    auto pd = [=](auto &cbd) { return is_matched(*cbd, event_base, event_id); };
    auto cbd = ranges::find_if(handlers_data, pd);
    if (cbd == handlers_data.end())
        return ESP_OK;

    cbd->get()->cnt = ++counter;
    UTIL_RETURN(DftEventLoop::get().post(event_base, event_id, event_data,
                                         event_data_size, ticks_to_wait));
    cbd->get()->eloop.get().mutex.take();
    return ESP_OK;
}

void MockDftEventLoop::handler_adapter(void *event_handler_arg,
                                       esp_event_base_t event_base,
                                       int32_t event_id, void *event_data) {
    static atomic<int> curr_counter{0};
    auto &cbd = *(cbdata *)event_handler_arg;
    if (cbd.cnt > curr_counter.exchange(cbd.cnt)) {
        proxy_callback(cbd, event_base, event_id, event_data);
        cbd.eloop.get().mutex.give();
    }
}
