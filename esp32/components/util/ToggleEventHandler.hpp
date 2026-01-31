#ifndef TOGGLE_EVENT_HANDLER_HPP
#define TOGGLE_EVENT_HANDLER_HPP

#include "util.hpp"
#include <vector>

class I_ToggleEventHandler : util::Toggle {
  public:
    using util::Toggle::is_on;
    using util::Toggle::off;
    using util::Toggle::on;
};

class ToggleEventHandler : public I_ToggleEventHandler {
  public:
    ToggleEventHandler(std::unique_ptr<util::EventHandler> &&handler);
    ~ToggleEventHandler() noexcept;
    esp_err_t on() override;
    esp_err_t off() override;
    bool is_on() const override;

  private:
    esp_event_handler_instance_t register_handler(const util::cbdata &cbdata);

    std::unique_ptr<util::EventHandler> handler;
    std::vector<util::cbdata> cbdata;
};

#endif