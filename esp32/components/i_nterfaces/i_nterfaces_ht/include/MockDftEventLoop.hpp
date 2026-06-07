#ifndef MOCK_DEFAULT_EVENT_LOOP_HPP
#define MOCK_DEFAULT_EVENT_LOOP_HPP

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
    int cnt;
};

class MockDftEventLoop {
  public:
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

    static MockDftEventLoop &get() {
        if (!instance)
            instance = std::make_unique<MockDftEventLoop>();
        return *instance;
    };
    static void reset() {
        if (instance)
            assert(instance->handlers_data.empty());
        instance.reset();
    };
    static MockDftEventLoop &reset_and_get() {
        reset();
        return get();
    };
    void wait_for_event() { mutex.take(); }

  private:
    static void handler_adapter(void *event_handler_arg,
                                esp_event_base_t event_base, int32_t event_id,
                                void *event_data);

    static std::unique_ptr<MockDftEventLoop> instance;

    util::Mutex mutex{};
    std::vector<std::unique_ptr<cbdata>> handlers_data{};
};

static_assert(I_DftEventLoop<MockDftEventLoop>);

#endif
