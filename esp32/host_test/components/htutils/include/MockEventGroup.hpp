#ifndef MOCK_EVENT_GROUP
#define MOCK_EVENT_GROUP

#include "util.hpp"
#include <gmock/gmock.h>

class MockEventGroup : public util::I_EventGroup {
  public:
    MOCK_METHOD(EventBits_t, clear, (const EventBits_t uxBitsToClear),
                (override));
    MOCK_METHOD(EventBits_t, get, (), (override));
    MOCK_METHOD(EventBits_t, set, (const EventBits_t uxBitsToSet), (override));
    MOCK_METHOD(EventBits_t, wait, (const EventBits_t uxBits), (override));
    MOCK_METHOD(EventBits_t, wait,
                (const EventBits_t uxBitsToWaitFor,
                 const BaseType_t xClearOnExit,
                 const BaseType_t xWaitForAllBits, TickType_t xTicksToWait),
                (override));
};

#endif
