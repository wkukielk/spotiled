#ifndef SERVER_COMMAND_HANDLER_HPP
#define SERVER_COMMAND_HANDLER_HPP

#include "ledaux.hpp"
#include "server/command.hpp"

template <I_LEDC LEDC> class CommandHandler {
  public:
    explicit CommandHandler(LEDC &ledc) : ledc{ledc} {};
    server::Response process(server::Message msg);

  protected:
    LEDC &ledc;

  private:
    server::Response handle(const server::FadeLed &);
};

template <typename T>
concept I_CommandHandler = requires(T t, server::Message msg) {
    { t.process(std::move(msg)) } -> std::same_as<server::Response>;
};

#include "server/command_handler.tpp"

#endif
