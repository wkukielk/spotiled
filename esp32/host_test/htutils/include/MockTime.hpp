#ifndef MOCK_TIME_HPP
#define MOCK_TIME_HPP

#include "time_aux.hpp"
#include <gmock/gmock.h>

class MockTime : public I_Time {
  public:
    MOCK_METHOD(std::unique_ptr<I_MiliTimer>, get_timer, (), (override));
    MOCK_METHOD(void, delay_ms, (int64_t delay), (override));
};

#endif
