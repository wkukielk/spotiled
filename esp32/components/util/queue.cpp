#include "queue.hpp"
#include "errors.hpp"
#include "util.hpp"

namespace util {
Queue::Queue(uint8_t length, size_t item_size)
    : handle(xQueueCreate(length, item_size)) {
    UTIL_THROW(handle == nullptr, InitError{"queue"});
}

Queue::~Queue() noexcept { vQueueDelete(handle); }

int Queue::receive(void *const data) { return receive(data, portMAX_DELAY); }
int Queue::receive_nowait(void *const data) { return receive(data, 0); }

int Queue::receive(void *const data, TickType_t wait) {
    return xQueueReceive(handle, data, wait) == pdTRUE ? ESP_OK : ESP_FAIL;
}

int Queue::send(void *data) { return send(data, portMAX_DELAY); }

int Queue::send(void *data, TickType_t wait) {
    return xQueueSend(handle, data, wait) == pdTRUE ? ESP_OK : ESP_FAIL;
}

PriorityQueue::PriorityQueue(uint8_t length, size_t item_size,
                             uint8_t n_queues) {
    UTIL_THROW(n_queues == 0, InitError{"n_queues is zero"});
    queues.reserve(n_queues);
    for (int i = 0; i != n_queues; ++i)
        queues.emplace_back(length, item_size);
}

int PriorityQueue::receive(void *const data) {
    return receive(data, portMAX_DELAY);
}

int PriorityQueue::receive(void *const data, TickType_t wait) {
    auto q = queues.rbegin();
    while (q < queues.rend() - prio && q->receive_nowait(data))
        q++;
    if (q == queues.rend() - prio)
        return queues.at(prio).receive(data, wait);
    return ESP_OK;
}

int PriorityQueue::receive_nowait(void *const data) { return receive(data, 0); }

int PriorityQueue::send(void *data, uint8_t prio) {
    return send(data, portMAX_DELAY, prio);
}

int PriorityQueue::send(void *data, TickType_t wait, uint8_t prio) {
    return queues.at(prio).send(data, wait);
}

void PriorityQueue::set_priority(uint8_t prio) {
    if (prio >= queues.size())
        throw std::runtime_error("too high priority set");
    this->prio = prio;
}
} // namespace util