#ifndef SERVER_COMMAND_HANDLER_HPP
#define SERVER_COMMAND_HANDLER_HPP

#include "animator.hpp"
#include "runtime_data.hpp"
#include "server/command.hpp"
#include "util.hpp"

template <I_Animatable LEDC> class CommandHandler {
  public:
    CommandHandler(LEDC &ledc, util::Notifiable<master::Data> &runtime_notifier)
        : ledc{ledc}, runtime_notifier{runtime_notifier} {};
    server::Response process(server::Message msg);

  protected:
    LEDC &ledc;
    util::Notifiable<master::Data> &runtime_notifier;

  private:
    server::Response handle(const server::FadeLed &);
    server::Response handle(const server::PulsateLed &);
    server::Response handle(const server::Reprovision &);
    server::Response handle(const server::RenameEsp &);

    template <typename D> static std::unique_ptr<master::Data> from_ws_server();
};

template <typename T>
concept I_CommandHandler = requires(T t, server::Message msg) {
    { t.process(std::move(msg)) } -> std::same_as<server::Response>;
};

#include "server/command_handler.tpp"

#endif
