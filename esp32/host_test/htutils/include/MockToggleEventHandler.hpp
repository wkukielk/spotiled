#ifndef MOCK_TOGGLE_EVENT_HANDLER_HPP
#define MOCK_TOGGLE_EVENT_HANDLER_HPP

#include "ToggleEventHandler.hpp"
#include <gmock/gmock.h>

class MockToggleEventHandler : public I_ToggleEventHandler {
  public:
    MOCK_METHOD(esp_err_t, on, (), (override));
    MOCK_METHOD(esp_err_t, off, (), (override));
    MOCK_METHOD(bool, is_on, (), (const, override));
};

#endif
