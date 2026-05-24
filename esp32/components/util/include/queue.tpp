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

template <typename T> void Queue<T>::receive(T *data) {
    receive(data, portMAX_DELAY);
}
template <typename T> int Queue<T>::receive_nowait(T *data) {
    return receive(data, 0);
}

template <typename T> int Queue<T>::receive(T *data, TickType_t wait) {
    return xQueueReceive(handle, data, wait) == pdTRUE ? ESP_OK : ESP_FAIL;
}

template <typename T> void Queue<T>::send(T *data) {
    send(data, portMAX_DELAY);
}

template <typename T> int Queue<T>::send(T *data, TickType_t wait) {
    return xQueueSendToBack(handle, data, wait) == pdTRUE ? ESP_OK : ESP_FAIL;
}

template <typename T> int Queue<T>::send_front(T *data) {
    return send_front(data, portMAX_DELAY);
}

template <typename T> int Queue<T>::send_front(T *data, TickType_t wait) {
    return xQueueSendToFront(handle, data, wait) == pdTRUE ? ESP_OK : ESP_FAIL;
}

template <typename T>
PointerQueue<T>::PointerQueue(uint8_t length) : queue{length} {}

template <typename T> std::unique_ptr<T> PointerQueue<T>::receive() {
    return receive(portMAX_DELAY);
}

template <typename T>
std::unique_ptr<T> PointerQueue<T>::receive(TickType_t wait) {
    T *out{};
    queue.receive(&out, wait);
    return std::unique_ptr<T>{out};
}

template <typename T> std::unique_ptr<T> PointerQueue<T>::receive_nowait() {
    return receive(0);
}

template <typename T>
SendStatus PointerQueue<T>::send(std::unique_ptr<T> &&data) {
    return send(std::move(data), portMAX_DELAY);
}

template <typename T>
SendStatus PointerQueue<T>::send(std::unique_ptr<T> &&data, TickType_t wait) {
    auto p = data.release();
    auto ret = queue.send(&p, wait) == 0;
    return ret ? SendStatus::success : SendStatus::full;
}
} // namespace util