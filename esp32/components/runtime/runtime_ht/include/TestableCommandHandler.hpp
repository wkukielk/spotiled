#ifndef TESTABLE_COMMAND_HANDLER_HPP
#define TESTABLE_COMMAND_HANDLER_HPP

#include "MockLEDC.hpp"
#include "server/command_handler.hpp"

class TestableCommandHandler : public CommandHandler<MockLEDC> {
  public:
    MockLEDC &get_ledc() { return ledc; }
};

#endif
