#ifndef TESTABLE_COMMAND_HANDLER_HPP
#define TESTABLE_COMMAND_HANDLER_HPP

#include "MockLEDCHolder.hpp"
#include "server/command_handler.hpp"

class TestableCommandHandler : private detail::MockLEDCHolder,
                               public CommandHandler<MockLEDC> {
  public:
    TestableCommandHandler() : CommandHandler{mock_ledc} {}
    MockLEDC &get_ledc() { return mock_ledc; }
};

#endif
