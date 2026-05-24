#ifndef UTIL_QUEUE_HPP
#define UTIL_QUEUE_HPP

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "freertos/FreeRTOS.h"
#pragma GCC diagnostic pop

#include <memory>

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
    int send_front(T *data);
    int send_front(T *data, TickType_t wait);

  private:
    QueueHandle_t handle;
};

enum class SendStatus { success, full };

template <typename T> class PointerQueue {
  public:
    PointerQueue(uint8_t length);
    std::unique_ptr<T> receive();
    std::unique_ptr<T> receive(TickType_t wait);
    std::unique_ptr<T> receive_nowait();
    SendStatus send(std::unique_ptr<T> &&data);
    SendStatus send(std::unique_ptr<T> &&data, TickType_t wait);

  private:
    Queue<T *> queue;
};

} // namespace util

#include "queue.tpp"

#endif
