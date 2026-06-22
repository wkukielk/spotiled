#include "TestableWSServer.hpp"
#include "WSServer.hpp"
#include <cstring>
#include <gtest/gtest.h>

using namespace std;
using namespace testing;

class WSServerTest : public testing::Test {
  protected:
    TestableWSServer server{};
    MockHTTPD &httpd = server.get_httpd();
};

TEST_F(WSServerTest, start) {
    EXPECT_CALL(httpd, start()).WillOnce(Return(ESP_OK));
    ASSERT_EQ(server.start(), ESP_OK);
}

TEST_F(WSServerTest, startFail) {
    EXPECT_CALL(httpd, start()).WillOnce(Return(ESP_FAIL));
    ASSERT_EQ(server.start(), ESP_FAIL);
}

TEST_F(WSServerTest, receive) {
    const std::string payload = "hello";
    httpd_req_t *req = (httpd_req_t *)1234;

    InSequence seq;
    EXPECT_CALL(httpd, ws_recv_frame_impl(req, _, 0))
        .WillOnce([&](httpd_req_t *, httpd_ws_frame_t *pkt, size_t) {
            pkt->len = payload.size();
            return ESP_OK;
        });
    EXPECT_CALL(httpd, ws_recv_frame_impl(req, _, payload.size()))
        .WillOnce([&](httpd_req_t *, httpd_ws_frame_t *pkt, size_t) {
            memcpy(pkt->payload, payload.data(), payload.size());
            return ESP_OK;
        });

    ASSERT_EQ(TestableWSServer::handler(req), ESP_OK);

    auto [buf, len] = server.receive_nowait();
    ASSERT_EQ(len, payload.size());
    ASSERT_STREQ(reinterpret_cast<char *>(buf.get()), payload.c_str());
}

TEST_F(WSServerTest, receiveFail) {
    const std::string payload = "hello";
    auto packetIsText =
        Pointee(Field(&httpd_ws_frame_t::type, HTTPD_WS_TYPE_TEXT));

    InSequence seq;
    EXPECT_CALL(httpd, ws_recv_frame_impl(_, packetIsText, 0))
        .WillOnce([&](httpd_req_t *, httpd_ws_frame_t *pkt, size_t) {
            pkt->len = payload.size();
            return ESP_OK;
        });
    EXPECT_CALL(httpd, ws_recv_frame_impl(_, packetIsText, payload.size()))
        .WillOnce([&](httpd_req_t *, httpd_ws_frame_t *pkt, size_t) {
            memcpy(pkt->payload, payload.data(), payload.size());
            return ESP_OK;
        });

    ASSERT_EQ(TestableWSServer::handler(nullptr), ESP_OK);

    auto [buf, len] = server.receive_nowait();
    ASSERT_EQ(len, payload.size());
    ASSERT_STREQ((char *)buf.get(), payload.c_str());
}
