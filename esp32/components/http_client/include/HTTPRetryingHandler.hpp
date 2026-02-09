#ifndef HTTP_RETRYING_HANDLER_HPP
#define HTTP_RETRYING_HANDLER_HPP

#include "HTTPHandler.hpp"

class I_MiliTimer;

class HTTPRetryingHandler : public HTTPHandler {
  public:
    HTTPRetryingHandler(std::unique_ptr<I_HTTPClient> &&client);
    http::code perform(CharBuf &buff) override;
};

#endif
