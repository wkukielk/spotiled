#ifndef MOCK_QUEUE_HPP
#define MOCK_QUEUE_HPP

#include "queue.hpp"
#include "runtime_data.hpp"
#include <gmock/gmock.h>

class MockQueue {
  public:
    MockQueue(uint8_t) {}

    MOCK_METHOD(std::unique_ptr<server::Data>, receive, (uint32_t), ());
    util::SendStatus send(std::unique_ptr<server::Data>) {
        return util::SendStatus::success;
    }
};

#endif
