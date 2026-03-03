#ifndef UTIL_QUEUE_HPP
#define UTIL_QUEUE_HPP

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "freertos/FreeRTOS.h"
#pragma GCC diagnostic pop

#include <vector>

namespace util {
class Queue {
  public:
    Queue(uint8_t length, size_t item_size);
    ~Queue() noexcept;
    int receive(void *const data);
    int receive(void *const data, TickType_t wait);
    int receive_nowait(void *const data);
    int send(void *data);
    int send(void *data, TickType_t wait);

  private:
    QueueHandle_t handle;
};

class PriorityQueue {
  public:
    PriorityQueue(uint8_t length, size_t item_size, uint8_t n_queues);
    int receive(void *const data);
    int receive(void *const data, TickType_t wait);
    int receive_nowait(void *const data);
    int send(void *data, uint8_t prio);
    int send(void *data, TickType_t wait, uint8_t prio);
    void set_priority(uint8_t prio);

  private:
    std::vector<Queue> queues;
    uint8_t prio = 0;
};
} // namespace util

#endif
