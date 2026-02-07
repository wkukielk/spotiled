#ifndef UTIL_HPP
#define UTIL_HPP

#define IGNORE_UNUSED_PUSH                                                     \
    _Pragma("GCC diagnostic push")                                             \
        _Pragma("GCC diagnostic ignored \"-Wunused-parameter\"")
#define IGNORE_UNUSED_POP _Pragma("GCC diagnostic pop")

IGNORE_UNUSED_PUSH
#include "freertos/FreeRTOS.h"
IGNORE_UNUSED_POP

#ifdef CONFIG_IDF_TARGET_LINUX
#include "bt_mock.hpp"
#else
#include "esp_debug_helpers.h"
#endif

#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "freertos/event_groups.h"
#include "straux.hpp"
#include <span>

#define UTIL_RETURN(ret)                                                       \
    do {                                                                       \
        auto err_rc_ = (ret);                                                  \
        if (unlikely(err_rc_ != ESP_OK))                                       \
            return err_rc_;                                                    \
    } while (0)

#define UTIL_THROW(ret, extype, ...)                                           \
    do {                                                                       \
        auto err_rc_ = (ret);                                                  \
        if (unlikely(err_rc_ != ESP_OK)) {                                     \
            esp_backtrace_print(5);                                            \
            throw extype(err_rc_, ##__VA_ARGS__);                              \
        }                                                                      \
    } while (0)

#define UTIL_LOGI(format, ...)                                                 \
    do {                                                                       \
        ESP_LOGI(TAG, format __VA_OPT__(, ) __VA_ARGS__);                      \
    } while (0)

#define UTIL_LOGW(format, ...)                                                 \
    do {                                                                       \
        ESP_LOGW(TAG, format __VA_OPT__(, ) __VA_ARGS__);                      \
    } while (0)

#define UTIL_LOGE(format, ...)                                                 \
    do {                                                                       \
        ESP_LOGE(TAG, format __VA_OPT__(, ) __VA_ARGS__);                      \
    } while (0)

namespace util {
class EventHandler;

struct cbdata {
    EventHandler *handler;
    esp_event_handler_t callback;
    esp_event_base_t base;
    int32_t id;
    esp_event_handler_instance_t instance;
    int cnt; // check out HandlerAdapter.cpp
};

enum class Register { YES, NO };

const EventBits_t fail_bit = 1 << 0;
const EventBits_t success_bit = 1 << 1;
const EventBits_t end_bit = 1 << 2;

class EventHandler {
  public:
    virtual ~EventHandler() {};
    virtual std::span<const util::cbdata> get_cbdata() const noexcept = 0;
};

class I_EventGroup {
  public:
    virtual ~I_EventGroup() noexcept {};
    virtual EventBits_t clear(const EventBits_t uxBitsToClear) = 0;
    virtual EventBits_t get() = 0;
    virtual EventBits_t set(const EventBits_t uxBitsToSet) = 0;
    virtual EventBits_t wait(const EventBits_t uxBits) = 0;
    virtual EventBits_t wait(const EventBits_t uxBitsToWaitFor,
                             const BaseType_t xClearOnExit,
                             const BaseType_t xWaitForAllBits,
                             TickType_t xTicksToWait) = 0;
};

class EventGroup : public I_EventGroup {
  public:
    EventGroup();
    ~EventGroup() noexcept override;
    EventBits_t clear(const EventBits_t uxBitsToClear) override;
    EventBits_t get() override;
    EventBits_t set(const EventBits_t uxBitsToSet) override;
    EventBits_t wait(const EventBits_t uxBits) override;
    EventBits_t wait(const EventBits_t uxBits, const BaseType_t xClearOnExit,
                     const BaseType_t xWaitForAllBits,
                     TickType_t xTicksToWait) override;

  private:
    EventGroupHandle_t handle;
};

class Toggle {
  public:
    virtual ~Toggle() {};
    virtual esp_err_t on() = 0;
    virtual esp_err_t off() = 0;
    virtual bool is_on() const = 0;
};

class HandlerProxy {
  public:
    static void callback(void *event_handler_arg, esp_event_base_t event_base,
                         int32_t event_id, void *event_data);
};

class I_Task {
  public:
    virtual ~I_Task() noexcept {};
    virtual void wakeup() = 0;
};

class Task : public I_Task {
  public:
    Task(TaskFunction_t fun, const char *name,
         const configSTACK_DEPTH_TYPE stack_depth, void *parameters,
         UBaseType_t priority);
    Task(TaskFunction_t fun, const char *name);
    ~Task() override;
    void wakeup() override;
    void sleep();

    TaskHandle_t handle;
    SemaphoreHandle_t s;
};

template <class T> constexpr inline bool in_between(T x, T a, T b) {
    return a <= x && x < b;
}

class IntWrap {
  public:
    IntWrap() = default;
    IntWrap(const IntWrap &) noexcept;
    IntWrap &operator=(const IntWrap &) noexcept;
    bool operator==(const IntWrap &other) const noexcept;
    operator int() const noexcept;

  protected:
    IntWrap(int id);
    int get_id() const noexcept;

  private:
    int id;
};

#define DECLARE_INT_WRAP_DERIVATIVE(derivative, name, id)                      \
    class name : public derivative {                                           \
      public:                                                                  \
        static const int i##d = id;                                            \
        name() : derivative{id} {}                                             \
    };
}; // namespace util

template <typename T> using u_ptr = std::unique_ptr<T>;

template <typename T, typename... Args> u_ptr<T> mkuptr(Args &&...args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

#endif
