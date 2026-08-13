#ifndef TESTABLE_SERVER_THREAD_HPP
#define TESTABLE_SERVER_THREAD_HPP

#include "MockCommandHandler.hpp"
#include "MockEventGroup.hpp"
#include "MockQueue.hpp"
#include "MockWSServer.hpp"
#include "server/server.hpp"

using namespace std;
using namespace testing;

struct TestServerTypes {
    using WSServer = MockWSServer;
    using Queue = NiceMock<MockQueue>;
    using Handler = MockCommandHandler;
};

class TestableServerThread : public ServerThread<TestServerTypes> {
  public:
    TestableServerThread(const util::JoinEventGroup &jg)
        : ServerThread{std::make_unique<MockCommandHandler>(), jg} {}

    using ServerThread::perform;
    MockWSServer &get_ws_server() { return ws_server; }
    MockCommandHandler &get_handler() { return *handler; }
    NiceMock<MockQueue> &get_queue() { return queue; }
};

#endif
