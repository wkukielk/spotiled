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
#include <functional>
#include <optional>
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

class EventHandler {
  public:
    virtual ~EventHandler() {};
    virtual std::span<const util::cbdata> get_cbdata() const noexcept = 0;
};

class EventGroupBase {
  public:
    virtual ~EventGroupBase() noexcept {};
    virtual EventBits_t clear(const EventBits_t uxBitsToClear) = 0;
    virtual EventBits_t get() = 0;
    virtual EventBits_t set(const EventBits_t uxBitsToSet) = 0;
    virtual EventBits_t wait(const EventBits_t uxBits) = 0;
    virtual EventBits_t wait(const EventBits_t uxBitsToWaitFor,
                             const BaseType_t xClearOnExit,
                             const BaseType_t xWaitForAllBits,
                             TickType_t xTicksToWait) = 0;
};

template <typename T>
concept I_EventGroup = requires(T t, const EventBits_t bits, const BaseType_t b,
                                TickType_t ticks) {
    { t.clear(bits) } -> std::same_as<EventBits_t>;
    { t.get() } -> std::same_as<EventBits_t>;
    { t.set(bits) } -> std::same_as<EventBits_t>;
    { t.wait(bits) } -> std::same_as<EventBits_t>;
    { t.wait(bits, b, b, ticks) } -> std::same_as<EventBits_t>;
};

class EventGroup : public EventGroupBase {
  public:
    EventGroup();
    EventGroup(EventGroup &&other) noexcept;
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

static_assert(I_EventGroup<EventGroup>);

template <typename F, typename ReturnType, typename... Args>
concept Returns = requires(F &&f, Args &&...args) {
    {
        std::forward<F>(f)(std::forward<Args>(args)...)
    } -> std::same_as<ReturnType>;
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

struct thread_params {
    TaskFunction_t fun{};
    std::string_view name;
    configSTACK_DEPTH_TYPE stack_depth;
    void *fun_parameters{};
    UBaseType_t priority;
    std::reference_wrapper<EventGroupBase> join_group;
    EventBits_t join_bit;
};

class Thread {
  public:
    Thread(thread_params &&params);
    void join() const noexcept { join_group.wait(join_bit); };
    static void wrapfn(void *params);

  private:
    TaskHandle_t handle{};
    TaskFunction_t fun{};
    void *fun_parameters{};
    EventGroupBase &join_group;
    EventBits_t join_bit{};
};

struct JoinEventGroup {
    std::reference_wrapper<EventGroupBase> egroup;
    EventBits_t join_bit;
};

class PostponedThread {
  public:
    PostponedThread(const JoinEventGroup &join_group);
    void join() { thread->join(); };

  protected:
    JoinEventGroup join_group;
    std::optional<Thread> thread;
};

class Mutex {
  public:
    Mutex();
    ~Mutex();
    void give();
    void take();

  private:
    SemaphoreHandle_t s{};
};

class I_Task {
  public:
    virtual ~I_Task() noexcept {};
    virtual void wakeup() = 0;
};

class Task : public I_Task {
  public:
    Task(Thread &&thread);
    void wakeup() override;
    void sleep();

    Thread thread;
    Mutex m;
};

template <typename T> class SingletonRef {
  public:
    T get() noexcept { return ref.load(); }
    void store(T x) noexcept {
        ++client_cnt;
        ref.store(x);
    }
    void drop() noexcept {
        if (--client_cnt == 0)
            ref.store(T{});
    }

  private:
    std::atomic<T> ref{};
    uint8_t client_cnt = 0;
};

template <typename Data> class Notifiable {
  public:
    virtual void notify(std::unique_ptr<Data> &&data) = 0;
};

template <class T> constexpr inline bool in_between(T x, T a, T b) {
    return a <= x && x < b;
}

esp_err_t retry(std::function<esp_err_t()> fun, unsigned retries);

}; // namespace util

template <typename T> using u_ptr = std::unique_ptr<T>;

template <typename T, typename... Args> u_ptr<T> mkuptr(Args &&...args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template <class... Ts> struct overloaded : Ts... {
    using Ts::operator()...;
};

#endif
