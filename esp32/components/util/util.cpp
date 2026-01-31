#include "util.hpp"
#include "errors.hpp"

namespace util {
EventGroup::EventGroup() {
    handle = xEventGroupCreate();
    UTIL_CHECK(handle == nullptr, "event group not created");
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

Task::Task(TaskFunction_t fun, const char *name,
           const configSTACK_DEPTH_TYPE stack_depth, void *parameters,
           UBaseType_t priority)
    : s{xSemaphoreCreateBinary()} {
    UTIL_THROW(s == NULL, InitError{"sem"});
    auto e = xTaskCreate(fun, name, stack_depth, parameters, priority, &handle);
    UTIL_THROW(e != pdPASS, InitError{"task"});
}

Task::Task(TaskFunction_t fun, const char *name) : s{xSemaphoreCreateBinary()} {
    UTIL_THROW(s == NULL, InitError{"sem"});
    auto e = xTaskCreate(fun, name, 4096, NULL, tskIDLE_PRIORITY, &handle);
    UTIL_THROW(e != pdPASS, InitError{"task"});
}

Task::~Task() {
    vTaskDelete(handle);
    vSemaphoreDelete(s);
}

void Task::sleep() { xSemaphoreTake(s, portMAX_DELAY); }

void Task::wakeup() { xSemaphoreGive(s); }

IntWrap::IntWrap(int id) : id{id} {}

IntWrap::IntWrap(const IntWrap &other) noexcept : id{other.id} {}
IntWrap &IntWrap::operator=(const IntWrap &other) noexcept {
    id = other.id;
    return *this;
}

bool IntWrap::operator==(const IntWrap &other) const noexcept {
    return get_id() == other.get_id();
}

IntWrap::operator int() const noexcept { return id; }

int IntWrap::get_id() const noexcept { return id; }
}; // namespace util
