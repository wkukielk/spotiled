#ifndef MOCK_COMMAND_HANDLER_HPP
#define MOCK_COMMAND_HANDLER_HPP

#include "server/command_handler.hpp"
#include <gmock/gmock.h>

class MockCommandHandler {
  public:
    MOCK_METHOD(server::Response, process, (server::Message msg), ());
};

static_assert(I_CommandHandler<MockCommandHandler>);

#endif
