#ifndef TESTABLE_WS_SERVER_HPP
#define TESTABLE_WS_SERVER_HPP

#include "MockHTTPD.hpp"
#include "WSServer.hpp"

class TestableWSServer : public WSServer<MockHTTPD> {
  public:
    MockHTTPD &get_httpd() noexcept { return httpd; }
    using WSServer<MockHTTPD>::handler;
};

#endif
