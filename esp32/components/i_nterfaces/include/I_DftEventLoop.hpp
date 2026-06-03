#ifndef DEFAULT_EVENT_LOOP
#define DEFAULT_EVENT_LOOP

#include "util.hpp"

IGNORE_UNUSED_PUSH
#include "esp_event.h"
IGNORE_UNUSED_POP

#include <concepts>
#include <optional>

template <typename T>
concept I_DftEventLoop = requires(T t) {
    { T::get() } -> std::same_as<T &>;
    {
        t.handler_register(esp_event_base_t{}, int32_t{}, esp_event_handler_t{},
                           (void *){}, (esp_event_handler_instance_t *){})
    } -> std::same_as<esp_err_t>;
    {
        t.handler_unregister(esp_event_base_t{}, int32_t{},
                             esp_event_handler_instance_t{})
    } -> std::same_as<esp_err_t>;
    {
        t.post(esp_event_base_t{}, int32_t{}, (const void *){}, size_t{},
               TickType_t{})
    } -> std::same_as<esp_err_t>;
};

class DftEventLoopConfigurer {
  public:
    DftEventLoopConfigurer();
    ~DftEventLoopConfigurer();
};

class DftEventLoop {
  public:
    esp_err_t
    handler_register(esp_event_base_t event_base, int32_t event_id,
                     esp_event_handler_t event_handler, void *event_handler_arg,
                     esp_event_handler_instance_t *instance) noexcept {
        return esp_event_handler_instance_register(
            event_base, event_id, event_handler, event_handler_arg, instance);
    }
    esp_err_t
    handler_unregister(esp_event_base_t event_base, int32_t event_id,
                       esp_event_handler_instance_t instance) noexcept {
        return esp_event_handler_instance_unregister(event_base, event_id,
                                                     instance);
    }
    esp_err_t post(esp_event_base_t event_base, int32_t event_id,
                   const void *event_data, size_t event_data_size,
                   TickType_t ticks_to_wait) noexcept {
        auto wait = ticks_to_wait;
        return esp_event_post(event_base, event_id, event_data, event_data_size,
                              wait);
    }
    static DftEventLoop &get() noexcept { return *instance; }
    static esp_err_t create() noexcept;
    static esp_err_t cleanup() noexcept;

  private:
    static std::optional<DftEventLoop> instance;
};

static_assert(I_DftEventLoop<DftEventLoop>);

#endif
