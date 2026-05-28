#ifndef WS_SERVER_HPP
#define WS_SERVER_HPP

#include "I_HTTPD.hpp"

template <I_HTTPD HTTPD> class WSServer {
  public:
    WSServer();
    esp_err_t start() noexcept;
    esp_err_t stop() noexcept;

  protected:
    HTTPD httpd;
};

#include "WSServer.tpp"

#endif
