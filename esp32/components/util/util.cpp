#include "util.hpp"
#include "errors.hpp"

namespace util {
using namespace std;

EventGroup::EventGroup() {
    handle = xEventGroupCreate();
    UTIL_THROW(handle == nullptr, InitError, "event group not created");
};

EventGroup::~EventGroup() noexcept { vEventGroupDelete(handle); }

EventBits_t EventGroup::clear(const EventBits_t uxBitsToClear) {
    return xEventGroupClearBits(handle, uxBitsToClear);
}

EventBits_t EventGroup::get() { return xEventGroupGetBits(handle); }

EventBits_t EventGroup::set(const EventBits_t uxBitsToSet) {
    return xEventGroupSetBits(handle, uxBitsToSet);
}

EventBits_t EventGroup::wait(const EventBits_t uxBits) {
    return xEventGroupWaitBits(handle, uxBits, pdFALSE, pdFALSE, portMAX_DELAY);
}

EventBits_t EventGroup::wait(const EventBits_t uxBits,
                             const BaseType_t xClearOnExit,
                             const BaseType_t xWaitForAllBits,
                             TickType_t xTicksToWait) {
    return xEventGroupWaitBits(handle, uxBits, xClearOnExit, xWaitForAllBits,
                               xTicksToWait);
}

void HandlerProxy::callback(void *event_handler_arg,
                            esp_event_base_t event_base, int32_t event_id,
                            void *event_data) {
    auto cbdata = static_cast<util::cbdata *>(event_handler_arg);
    (*cbdata->callback)(cbdata, event_base, event_id, event_data);
}

Thread::Thread(thread_params &&params)
    : fun{move(params.fun)}, fun_parameters{move(params.fun_parameters)},
      join_group{*params.join_group}, join_bit{move(params.join_bit)} {
    auto name = move(params.name);
    auto stack_depth = params.stack_depth ? move(params.stack_depth) : 4096;
    auto priority = params.priority ? move(params.priority) : tskIDLE_PRIORITY;
    auto e = xTaskCreate(wrapfun, name, stack_depth, this, priority, &handle);
    UTIL_THROW(e != pdPASS, InitError, "task");
}

void Thread::join() const noexcept { join_group.wait(join_bit); }

void Thread::wrapfun(void *params) {
    auto &thread = *(Thread *)params;
    (*thread.fun)(thread.fun_parameters);
    thread.join_group.set(thread.join_bit);
    vTaskDelete(NULL);
}

Mutex::Mutex() : s{xSemaphoreCreateBinary()} {
    UTIL_THROW(s == NULL, InitError, "sem");
}

Mutex::~Mutex() { vSemaphoreDelete(s); }

void Mutex::give() { xSemaphoreGive(s); }

void Mutex::take() { xSemaphoreTake(s, portMAX_DELAY); }

Task::Task(Thread &&thread) : thread{move(thread)}, m{Mutex{}} {}

void Task::sleep() { m.take(); }

void Task::wakeup() { m.give(); }

esp_err_t retry(std::function<esp_err_t()> fun, unsigned retries) {
    auto status = esp_err_t{ESP_FAIL};
    while (retries-- && status != ESP_OK)
        status = fun();
    return status;
}
}; // namespace util
