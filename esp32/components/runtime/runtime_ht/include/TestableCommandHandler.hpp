#ifndef TESTABLE_COMMAND_HANDLER_HPP
#define TESTABLE_COMMAND_HANDLER_HPP

#include "MockLEDCHolder.hpp"
#include "MockRuntimeNotifiable.hpp"
#include "server/command_handler.hpp"

class TestableCommandHandler : private detail::MockLEDCHolder,
                               public CommandHandler<MockLEDC> {
  public:
    explicit TestableCommandHandler(
        util::Notifiable<master::Data> &runtime_notifier)
        : CommandHandler{mock_ledc, runtime_notifier} {}
    MockLEDC &get_ledc() { return mock_ledc; }
};

#endif
