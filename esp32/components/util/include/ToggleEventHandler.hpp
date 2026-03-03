#ifndef TOGGLE_EVENT_HANDLER_HPP
#define TOGGLE_EVENT_HANDLER_HPP

#include "util.hpp"
#include <functional>
#include <vector>

class I_ToggleEventHandler : util::Toggle {
  public:
    using util::Toggle::is_on;
    using util::Toggle::off;
    using util::Toggle::on;
};

class ToggleEventHandler : public I_ToggleEventHandler {
  public:
    ToggleEventHandler(
        std::function<std::unique_ptr<util::EventHandler>()> get_handler);
    ~ToggleEventHandler() noexcept;
    esp_err_t on() override;
    esp_err_t off() override;
    bool is_on() const override;

    std::function<std::unique_ptr<util::EventHandler>()> get_handler{};
    std::unique_ptr<util::EventHandler> handler{};
};

#endif