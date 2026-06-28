#ifndef MOCK_DEFAULT_EVENT_LOOP_HPP
#define MOCK_DEFAULT_EVENT_LOOP_HPP

#include "Handle.hpp"
#include "I_DftEventLoop.hpp"
#include <functional>
#include <gmock/gmock.h>

class MockDftEventLoop;

struct cbdata {
    esp_event_base_t base;
    int32_t id;
    esp_event_handler_t handler;
    void *handler_arg;
    esp_event_handler_instance_t instance;
    std::reference_wrapper<MockDftEventLoop> eloop;
};

class MockDftEventLoop {
  public:
    esp_err_t init() noexcept { return ESP_OK; }
    esp_err_t deinit() noexcept { return ESP_OK; }
    esp_err_t handler_register(esp_event_base_t event_base, int32_t event_id,
                               esp_event_handler_t event_handler,
                               void *event_handler_arg,
                               esp_event_handler_instance_t *instance) noexcept;
    esp_err_t
    handler_unregister(esp_event_base_t event_base, int32_t event_id,
                       esp_event_handler_instance_t instance) noexcept;
    esp_err_t post(esp_event_base_t event_base, int32_t event_id,
                   const void *event_data, size_t event_data_size,
                   TickType_t ticks_to_wait) noexcept;
    esp_err_t post(esp_event_base_t event_base, int32_t event_id,
                   const void *event_data, size_t event_data_size) noexcept {
        return post(event_base, event_id, event_data, event_data_size,
                    portMAX_DELAY);
    }

    void wait_for_event() { mutex.take(); }

  private:
    static void handler_adapter(void *event_handler_arg,
                                esp_event_base_t event_base, int32_t event_id,
                                void *event_data);

    Handle<DftEventLoop> eloop{};
    util::Mutex mutex{};
    std::vector<std::unique_ptr<cbdata>> handlers_data{};
};

static_assert(I_DftEventLoop<MockDftEventLoop>);

#endif
