#ifndef TESTABLE_UDP_SERVER_HPP
#define TESTABLE_UDP_SERVER_HPP

#include "MockUDPSocket.hpp"
#include "UDPServer.hpp"

class TestableUDPServer : public UDPServer<MockUDPSocket> {
  public:
    MockUDPSocket &get_socket() noexcept { return socket; }
};

#endif
