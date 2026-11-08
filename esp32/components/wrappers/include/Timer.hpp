#ifndef WRAPPERS_TIMER_HPP
#define WRAPPERS_TIMER_HPP

#include "I_Timer.hpp"
#include "esp_timer.h"
#include <chrono>

class Timer {
  public:
    Timer(timer::callback cb, const char *name);
    ~Timer() noexcept;

    Timer(const Timer &) = delete;
    Timer &operator=(const Timer &) = delete;
    Timer(Timer &&) = delete;
    Timer &operator=(Timer &&) = delete;

    esp_err_t start_once(std::chrono::microseconds timeout) noexcept;
    esp_err_t start_periodic(std::chrono::microseconds period) noexcept;
    esp_err_t restart(std::chrono::microseconds timeout) noexcept;
    esp_err_t stop() noexcept;
    bool is_active() const noexcept;

  private:
    static void dispatch(void *arg);

    timer::callback cb;
    esp_timer_handle_t handle{};
};

static_assert(I_Timer<Timer>);

#endif
