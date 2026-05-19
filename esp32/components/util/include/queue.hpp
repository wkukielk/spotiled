#ifndef UTIL_QUEUE_HPP
#define UTIL_QUEUE_HPP

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "freertos/FreeRTOS.h"
#pragma GCC diagnostic pop

namespace util {
template <typename T> class Queue {
  public:
    Queue(uint8_t length);
    ~Queue() noexcept;
    int receive(void *const data);
    int receive(void *const data, TickType_t wait);
    int receive_nowait(void *const data);
    int send(void *data);
    int send(void *data, TickType_t wait);

  private:
    QueueHandle_t handle;
};
} // namespace util

#include "queue.tpp"

#endif
