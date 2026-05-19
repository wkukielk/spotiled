#if !defined(UTIL_QUEUE_HPP)
#include "queue.hpp"
#endif

#include "errors.hpp"
#include "util.hpp"

namespace util {
template <typename T>
Queue<T>::Queue(uint8_t length) : handle(xQueueCreate(length, sizeof(T))) {
    UTIL_THROW(handle == nullptr, InitError, "queue");
}

template <typename T> Queue<T>::~Queue() noexcept { vQueueDelete(handle); }

template <typename T> int Queue<T>::receive(void *const data) {
    return receive(data, portMAX_DELAY);
}
template <typename T> int Queue<T>::receive_nowait(void *const data) {
    return receive(data, 0);
}

template <typename T> int Queue<T>::receive(void *const data, TickType_t wait) {
    return xQueueReceive(handle, data, wait) == pdTRUE ? ESP_OK : ESP_FAIL;
}

template <typename T> int Queue<T>::send(void *data) {
    return send(data, portMAX_DELAY);
}

template <typename T> int Queue<T>::send(void *data, TickType_t wait) {
    return xQueueSend(handle, data, wait) == pdTRUE ? ESP_OK : ESP_FAIL;
}
} // namespace util