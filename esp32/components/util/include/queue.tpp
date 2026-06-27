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

template <typename T>
Queue<T>::Queue(Queue &&other) noexcept : handle{other.handle} {
    other.handle = nullptr;
}

template <typename T> Queue<T>::~Queue() noexcept {
    if (handle)
        vQueueDelete(handle);
}

template <typename T> inline void Queue<T>::receive(T *data) {
    receive(data, portMAX_DELAY);
}
template <typename T> inline int Queue<T>::receive_nowait(T *data) {
    return receive(data, 0);
}

template <typename T> inline int Queue<T>::receive(T *data, TickType_t wait) {
    return xQueueReceive(handle, data, wait) == pdTRUE ? ESP_OK : ESP_FAIL;
}

template <typename T> inline void Queue<T>::send(T *data) {
    send(data, portMAX_DELAY);
}

template <typename T> inline int Queue<T>::send(T *data, TickType_t wait) {
    return xQueueSendToBack(handle, data, wait) == pdTRUE ? ESP_OK : ESP_FAIL;
}

template <typename T> inline int Queue<T>::send_nowait(T *data) {
    return send(data, 0);
}

template <typename T> inline int Queue<T>::send_front(T *data) {
    return send_front(data, portMAX_DELAY);
}

template <typename T> int Queue<T>::send_front(T *data, TickType_t wait) {
    return xQueueSendToFront(handle, data, wait) == pdTRUE ? ESP_OK : ESP_FAIL;
}

template <typename T> inline int Queue<T>::send_front_nowait(T *data) {
    return send_front(data, 0);
}

template <typename T>
PointerQueue<T>::PointerQueue(uint8_t length) : queue{length} {}

template <typename T> inline std::unique_ptr<T> PointerQueue<T>::receive() {
    return receive(portMAX_DELAY);
}

template <typename T>
std::unique_ptr<T> PointerQueue<T>::receive(uint32_t wait) {
    T *out{};
    queue.receive(&out, pdMS_TO_TICKS(wait));
    return std::unique_ptr<T>{out};
}

template <typename T>
inline std::unique_ptr<T> PointerQueue<T>::receive_nowait() {
    return receive(0);
}

template <typename T>
SendStatus inline PointerQueue<T>::send(std::unique_ptr<T> &&data) {
    return send(std::move(data), pdTICKS_TO_MS(portMAX_DELAY));
}

template <typename T>
SendStatus PointerQueue<T>::send(std::unique_ptr<T> &&data, uint64_t wait) {
    auto p = data.release();
    auto success = queue.send(&p, pdMS_TO_TICKS(wait)) == 0;
    if (!success)
        std::default_delete<T>{}(p);
    return success ? SendStatus::success : SendStatus::full;
}

template <typename T>
inline SendStatus PointerQueue<T>::send_nowait(std::unique_ptr<T> &&data) {
    return send(std::move(data), 0);
}

template <typename T>
ArrayQueue<T>::ArrayQueue(uint8_t length) : queue{length} {}

template <typename T>
std::pair<std::unique_ptr<T[]>, size_t> ArrayQueue<T>::receive() {
    return receive(portMAX_DELAY);
}

template <typename T>
std::pair<std::unique_ptr<T[]>, size_t> ArrayQueue<T>::receive(uint32_t wait) {
    std::span<T> out{};
    queue.receive(&out, pdMS_TO_TICKS(wait));
    return {std::unique_ptr<T[]>{out.data()}, out.size()};
}

template <typename T>
std::pair<std::unique_ptr<T[]>, size_t> ArrayQueue<T>::receive_nowait() {
    return receive(0);
}

template <typename T> SendStatus ArrayQueue<T>::send(Message &&data) {
    return send(std::move(data), pdTICKS_TO_MS(portMAX_DELAY));
}

template <typename T>
SendStatus ArrayQueue<T>::send(Message &&data, uint64_t wait) {
    auto elem = std::span<T>{data.first.release(), data.second};
    auto success = queue.send(&elem, pdMS_TO_TICKS(wait)) == 0;
    if (!success)
        std::default_delete<T[]>{}(elem.data());
    return success ? SendStatus::success : SendStatus::full;
}

template <typename T> SendStatus ArrayQueue<T>::send_nowait(Message &&data) {
    return send(std::move(data), 0);
}

} // namespace util