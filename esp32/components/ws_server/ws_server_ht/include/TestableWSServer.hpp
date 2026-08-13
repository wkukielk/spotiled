#ifndef TESTABLE_WS_SERVER_HPP
#define TESTABLE_WS_SERVER_HPP

#include "MockHTTPD.hpp"
#include "WSServer.hpp"

class TestableWSServer : public WSServer<::testing::NiceMock<MockHTTPD>> {
  public:
    ::testing::NiceMock<MockHTTPD> &get_httpd() noexcept { return httpd; }
    using WSServer<::testing::NiceMock<MockHTTPD>>::handler;
};

#endif
