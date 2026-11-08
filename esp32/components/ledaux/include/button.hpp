#ifndef LEDAUX_BUTTON_HPP
#define LEDAUX_BUTTON_HPP

#include "I_GPIO.hpp"
#include "I_Timer.hpp"
#include <atomic>

enum class btn_press { no, short_, long_ };

template <typename T>
concept ButtonTypes = requires {
    requires I_GPIO_IN<typename T::GPIO>;
    requires I_PeriodicTimer<typename T::PeriodicTimer>;
    requires I_OneShotTimer<typename T::OneShotTimer>;
};

template <ButtonTypes T> class Button {
  public:
    Button();
    btn_press get_last_press() noexcept;

  private:
    void probe() noexcept;
    void submit_long_press() noexcept;

    T::GPIO gpio;
    std::atomic<btn_press> pending_notify{btn_press::no};

    bool is_pressed = false;
    T::PeriodicTimer probe_timer;
    T::OneShotTimer press_timer;

    static constexpr std::chrono::milliseconds poll_period{50};
    static constexpr std::chrono::seconds short_period{1};
    static constexpr std::chrono::seconds long_period{5};
};

#include "button.tpp"

#endif
