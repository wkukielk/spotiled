#include <chrono>

template <ButtonTypes T>
Button<T>::Button()
    : gpio{GPIO_NUM_2, gpio::pull::up},
      probe_timer{[this] { probe(); }, "btn-probe"},
      press_timer{[this] { submit_long_press(); }, "btn-press"} {
    probe_timer.start(poll_period);
}

template <ButtonTypes T> btn_press Button<T>::get_last_press() noexcept {
    return pending_notify.exchange(btn_press::no);
}

template <ButtonTypes T> void Button<T>::probe() noexcept {
    bool new_is_pressed = gpio.get_level() == gpio::level::low;
    if (new_is_pressed && !is_pressed)
        press_timer.start(long_period);
    else if (!new_is_pressed && is_pressed && press_timer.is_active()) {
        if (press_timer.remaining() <= long_period - short_period) {
            auto expected = btn_press::no;
            pending_notify.compare_exchange_strong(expected, btn_press::short_);
        }
        press_timer.stop();
    }
    is_pressed = new_is_pressed;
}

template <ButtonTypes T> void Button<T>::submit_long_press() noexcept {
    auto expected_no = btn_press::no;
    pending_notify.compare_exchange_strong(expected_no, btn_press::long_);
}
