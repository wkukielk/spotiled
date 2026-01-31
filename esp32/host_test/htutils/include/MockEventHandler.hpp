#ifndef MOCK_EVENT_HANDLER
#define MOCK_EVENT_HANDLER

#include "util.hpp"
#include <gmock/gmock.h>

class MockEventHandler : public util::EventHandler {
  public:
    MOCK_METHOD(std::span<const util::cbdata>, get_cbdata, (),
                (const, noexcept, override));
};

#endif
