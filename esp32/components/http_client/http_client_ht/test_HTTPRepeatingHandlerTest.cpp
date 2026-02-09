#include "BaseTest.hpp"
#include "HTTPHandlerTestAux.hpp"
#include "HTTPRetryingHandler.hpp"
#include "MockHTTPClient.hpp"
#include "esp_event.h"
#include "httpaux.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

class HTTPRepeatingHandlerTest : public BaseTest {
  protected:
    HTTPRepeatingHandlerTest() {
        auto client = std::make_unique<MockHTTPClient>();
        handler = std::make_unique<HTTPRetryingHandler>(std::move(client));
        this->client = (MockHTTPClient *)&handler->get_client();
        aux = std::make_unique<HTTPHandlerTestAux>(*handler);
    }
    MockHTTPClient *client;
    std::unique_ptr<HTTPRetryingHandler> handler;
    std::unique_ptr<HTTPHandlerTestAux> aux;
    CharBuf buf{};
};

TEST_F(HTTPRepeatingHandlerTest, success) {
    auto data = esp_http_client_event{
        .event_id = HTTP_EVENT_ON_DATA, .data = (void *)"1234", .data_len = 4};
    EXPECT_CALL(*client, is_chunked_response()).WillOnce(Return(false));
    EXPECT_CALL(*client, get_content_length()).WillOnce(Return(4));
    aux->post(data);
    auto finish_data = esp_http_client_event{.event_id = HTTP_EVENT_ON_FINISH};
    aux->post(finish_data);
    EXPECT_CALL(*client, perform()).WillOnce(Return(ESP_OK));
    EXPECT_CALL(*client, get_status_code()).WillOnce(Return(http::code::OK));
    handler->perform(buf);
    EXPECT_STREQ(buf.data(), "1234");
    EXPECT_EQ(buf.size(), 5);
}

TEST_F(HTTPRepeatingHandlerTest, connectionProblems) {
    auto data = esp_http_client_event{
        .event_id = HTTP_EVENT_ON_DATA, .data = (void *)"1234", .data_len = 4};
    EXPECT_CALL(*client, is_chunked_response()).WillOnce(Return(false));
    EXPECT_CALL(*client, get_content_length()).WillOnce(Return(4));
    aux->post(data);
    auto finish_data = esp_http_client_event{.event_id = HTTP_EVENT_ON_FINISH};
    aux->post(finish_data);
    EXPECT_CALL(*client, perform())
        .Times(2)
        .WillOnce(Return(http::code::ERR_CONNECT))
        .WillOnce(Return(ESP_OK));
    EXPECT_CALL(*time, delay_ms(_));
    EXPECT_CALL(*client, get_status_code()).WillOnce(Return(http::code::OK));
    EXPECT_EQ(handler->perform(buf), http::code::OK);
    EXPECT_STREQ(buf.data(), "1234");
    EXPECT_EQ(buf.size(), 5);
}

TEST_F(HTTPRepeatingHandlerTest, tooMuchConnectionProblems) {
    auto data = esp_http_client_event{
        .event_id = HTTP_EVENT_ON_DATA, .data = (void *)"1234", .data_len = 4};
    EXPECT_CALL(*client, is_chunked_response()).WillOnce(Return(false));
    EXPECT_CALL(*client, get_content_length()).WillOnce(Return(4));
    aux->post(data);
    auto finish_data = esp_http_client_event{.event_id = HTTP_EVENT_ON_FINISH};
    aux->post(finish_data);
    EXPECT_CALL(*client, perform())
        .Times(5)
        .WillRepeatedly(Return(http::code::ERR_CONNECT));
    EXPECT_CALL(*time, delay_ms(_)).Times(5);
    EXPECT_EQ(handler->perform(buf), http::code::ERR_CONNECT);
    EXPECT_TRUE(buf.empty());
}
