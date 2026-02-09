#include "BaseTest.hpp"
#include "HTTPHandlerTestAux.hpp"
#include "MockHTTPClient.hpp"
#include "esp_event.h"
#include "httpaux.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

class HTTPHandlerTest : public BaseTest {
  protected:
    HTTPHandlerTest() {
        auto client = std::make_unique<MockHTTPClient>();
        handler = std::make_unique<HTTPHandler>(std::move(client));
        this->client = (MockHTTPClient *)&handler->get_client();
        aux = std::make_unique<HTTPHandlerTestAux>(*handler);
    }
    MockHTTPClient *client;
    std::unique_ptr<HTTPHandler> handler;
    std::unique_ptr<HTTPHandlerTestAux> aux;
    CharBuf buf{};
};

TEST_F(HTTPHandlerTest, HeadersEmpty) {
    auto headers = handler->get_headers();
    EXPECT_EQ(headers.size(), 0);
}

TEST_F(HTTPHandlerTest, ReceiveHeader) {
    auto header_data = esp_http_client_event{.event_id = HTTP_EVENT_ON_HEADER,
                                             .header_key = "User-Agent",
                                             .header_value = "ODRA"};
    aux->post(header_data);
    auto finish_data = esp_http_client_event{.event_id = HTTP_EVENT_ON_FINISH};
    aux->post(finish_data);
    auto headers = handler->get_headers();
    EXPECT_EQ(headers.at("User-Agent"), "ODRA");
    EXPECT_EQ(headers.size(), 1);
}

TEST_F(HTTPHandlerTest, HeadersReset) {
    auto header_data = esp_http_client_event{.event_id = HTTP_EVENT_ON_HEADER,
                                             .header_key = "User-Agent",
                                             .header_value = "ODRA"};
    aux->post(header_data);
    auto finish_data = esp_http_client_event{.event_id = HTTP_EVENT_ON_FINISH};
    aux->post(finish_data);
    header_data.header_key = "Content-Type";
    header_data.header_value = "video/mpeg";
    aux->post(header_data);
    auto headers = handler->get_headers();
    EXPECT_EQ(headers.size(), 1);
}

TEST_F(HTTPHandlerTest, ReceiveData) {
    auto data = esp_http_client_event{
        .event_id = HTTP_EVENT_ON_DATA, .data = (void *)"1234", .data_len = 4};
    EXPECT_CALL(*client, is_chunked_response()).WillRepeatedly(Return(false));
    EXPECT_CALL(*client, get_content_length()).WillOnce(Return(7));
    aux->post(data);
    data.data = (void *)"567";
    data.data_len = 3;
    aux->post(data);
    auto finish_data = esp_http_client_event{.event_id = HTTP_EVENT_ON_FINISH};
    aux->post(finish_data);
    EXPECT_EQ(handler->release(buf), ESP_OK);
    EXPECT_STREQ(buf.data(), "1234567");
    EXPECT_EQ(buf.size(), 8);
}

TEST_F(HTTPHandlerTest, DataResets) {
    auto data = esp_http_client_event{
        .event_id = HTTP_EVENT_ON_DATA, .data = (void *)"1234", .data_len = 4};
    EXPECT_CALL(*client, is_chunked_response()).WillRepeatedly(Return(false));
    EXPECT_CALL(*client, get_content_length()).WillOnce(Return(7));
    aux->post(data);
    auto finish_data = esp_http_client_event{.event_id = HTTP_EVENT_ON_FINISH};
    aux->post(finish_data);
    EXPECT_CALL(*client, get_content_length()).WillOnce(Return(4));
    aux->post(data);
    aux->post(finish_data);
    handler->release(buf);
    EXPECT_STREQ(buf.data(), "1234");
    EXPECT_EQ(buf.size(), 5);
}

TEST_F(HTTPHandlerTest, ChunkedResponse) {
    auto data = esp_http_client_event{.event_id = HTTP_EVENT_ON_DATA};
    EXPECT_CALL(*client, is_chunked_response()).WillOnce(Return(true));
    aux->post(data);
}

TEST_F(HTTPHandlerTest, releaseNothing) {
    EXPECT_EQ(handler->release(buf), http::code::ERR_NO_PAYLOAD);
    EXPECT_TRUE(!buf.data());
    EXPECT_TRUE(buf.size() == 0);
}

TEST_F(HTTPHandlerTest, performNoContent) {
    EXPECT_CALL(*client, perform()).WillOnce(Return(ESP_OK));
    EXPECT_CALL(*client, get_status_code())
        .WillOnce(Return(http::code::NO_CONTENT));
    EXPECT_EQ(handler->perform(buf), http::code::NO_CONTENT);
}

TEST_F(HTTPHandlerTest, performSomeError) {
    EXPECT_CALL(*client, perform()).WillOnce(Return(ESP_OK));
    EXPECT_CALL(*client, get_status_code()).WillOnce(Return(http::code::OK));
    EXPECT_EQ(handler->perform(buf), http::code::ERR_NO_PAYLOAD);
}

TEST_F(HTTPHandlerTest, performOK) {
    auto data = esp_http_client_event{
        .event_id = HTTP_EVENT_ON_DATA, .data = (void *)"1234", .data_len = 4};
    EXPECT_CALL(*client, is_chunked_response()).WillOnce(Return(false));
    EXPECT_CALL(*client, get_content_length()).WillOnce(Return(4));
    aux->post(data);
    auto finish_data = esp_http_client_event{.event_id = HTTP_EVENT_ON_FINISH};
    aux->post(finish_data);
    EXPECT_CALL(*client, perform()).WillOnce(Return(ESP_OK));
    EXPECT_CALL(*client, get_status_code()).WillOnce(Return(http::code::OK));
    EXPECT_EQ(handler->perform(buf), http::code::OK);
    EXPECT_STREQ(buf.data(), "1234");
    EXPECT_EQ(buf.size(), 5);
}

TEST_F(HTTPHandlerTest, perform401) {
    auto data = esp_http_client_event{
        .event_id = HTTP_EVENT_ON_DATA, .data = (void *)"1234", .data_len = 4};
    EXPECT_CALL(*client, is_chunked_response()).WillOnce(Return(false));
    EXPECT_CALL(*client, get_content_length()).WillOnce(Return(4));
    aux->post(data);
    auto finish_data = esp_http_client_event{.event_id = HTTP_EVENT_ON_FINISH};
    aux->post(finish_data);
    EXPECT_CALL(*client, perform()).WillOnce(Return(ESP_OK));
    EXPECT_CALL(*client, get_status_code())
        .WillOnce(Return(http::code::UNAUTHORIZED));
    EXPECT_EQ(handler->perform(buf), http::code::UNAUTHORIZED);
    EXPECT_STREQ(buf.data(), "1234");
    EXPECT_EQ(buf.size(), 5);
}

TEST_F(HTTPHandlerTest, connectionError) {
    EXPECT_CALL(*client, perform()).WillOnce(Return(http::code::ERR_CONNECT));
    EXPECT_EQ(handler->perform(buf), http::code::ERR_CONNECT);
    EXPECT_TRUE(!buf.data());
    EXPECT_TRUE(buf.size() == 0);
}
