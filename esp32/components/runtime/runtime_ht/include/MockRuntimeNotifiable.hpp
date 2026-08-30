#ifndef MOCK_RUNTIME_NOTIFIABLE_HPP
#define MOCK_RUNTIME_NOTIFIABLE_HPP

#include "runtime_data.hpp"
#include "util.hpp"
#include <gmock/gmock.h>

class MockRuntimeNotifiable : public util::Notifiable<master::Data> {
  public:
    MOCK_METHOD(void, notify, (std::unique_ptr<master::Data> && data),
                (override));
};

#endif
