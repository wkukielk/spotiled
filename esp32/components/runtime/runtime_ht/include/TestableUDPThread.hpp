#ifndef TESTABLE_UDP_THREAD_HPP
#define TESTABLE_UDP_THREAD_HPP

#include "MockEventGroup.hpp"
#include "MockLedColorHandler.hpp"
#include "MockUDPServer.hpp"
#include "server/udp_thread.hpp"

struct TestUDPThreadTypes {
    using UDPServer = MockUDPServer;
    using Handler = MockLedColorHandler;
};

class TestableUDPThread : public UDPThread<TestUDPThreadTypes> {
  public:
    TestableUDPThread(const util::JoinEventGroup &jg)
        : UDPThread{std::make_unique<MockLedColorHandler>(), jg} {}

    using UDPThread::perform;
    MockUDPServer &get_udp_server() { return udp_server; }
    MockLedColorHandler &get_handler() { return *handler; }
};

#endif
