#include "TestableWSServer.hpp"
#include "WSServer.hpp"
#include <gtest/gtest.h>

using namespace std;
using namespace testing;

class WSServerTest : public testing::Test {
  protected:
    WSServerTest() : server{}, httpd{&server.get_httpd()} {}

    TestableWSServer server;
    MockHTTPD *httpd{};
};

TEST_F(WSServerTest, init) { ASSERT_EQ(1, 0); }
