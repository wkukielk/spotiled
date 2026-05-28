#ifndef UTIL_QUEUE_HPP
#define UTIL_QUEUE_HPP

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "freertos/FreeRTOS.h"
#pragma GCC diagnostic pop

#include <memory>
#include <span>

namespace util {
template <typename T> class Queue {
  public:
    Queue(uint8_t length);
    ~Queue() noexcept;
    void receive(T *data);
    int receive(T *data, TickType_t wait);
    int receive_nowait(T *data);
    void send(T *data);
    int send(T *data, TickType_t wait);
    int send_nowait(T *data);
    int send_front(T *data);
    int send_front(T *data, TickType_t wait);
    int send_front_nowait(T *data);

  private:
    QueueHandle_t handle;
};

enum class SendStatus { success, full };

template <typename T> class PointerQueue {
  public:
    PointerQueue(uint8_t length);
    std::unique_ptr<T> receive();
    std::unique_ptr<T> receive(uint32_t wait);
    std::unique_ptr<T> receive_nowait();
    SendStatus send(std::unique_ptr<T> &&data);
    SendStatus send(std::unique_ptr<T> &&data, uint64_t wait);
    SendStatus send_nowait(std::unique_ptr<T> &&data);

  private:
    Queue<T *> queue;
};

template <typename T> class ArrayQueue {
  public:
    using Message = std::pair<std::unique_ptr<T[]>, size_t>;
    ArrayQueue(uint8_t length);
    Message receive();
    Message receive(uint32_t wait);
    Message receive_nowait();
    SendStatus send(Message &&data);
    SendStatus send(Message &&data, uint64_t wait);
    SendStatus send_nowait(Message &&data);

  private:
    Queue<std::span<T>> queue;
};

} // namespace util

#include "queue.tpp"

#endif
