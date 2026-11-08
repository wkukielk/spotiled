#ifndef LEDAUX_HT_FAKE_BUTTON_DEPS_HPP
#define LEDAUX_HT_FAKE_BUTTON_DEPS_HPP

#include "I_GPIO.hpp"
#include "I_Timer.hpp"

#include <chrono>
#include <gmock/gmock.h>
#include <optional>
#include <utility>

class FakeGPIO {
  public:
    FakeGPIO(gpio_num_t pin, gpio::pull pull) : pin{pin}, pull{pull} {
        instance = this;
    }
    ~FakeGPIO() { instance = nullptr; }

    gpio::level get_level() const noexcept { return level; }

    inline static FakeGPIO *instance = nullptr;
    gpio_num_t pin;
    gpio::pull pull;
    gpio::level level = gpio::level::high;
};

template <typename Self> class FakeTimerBase {
  public:
    FakeTimerBase(timer::callback cb, const char *name)
        : cb{std::move(cb)}, name{name} {
        instance = static_cast<Self *>(this);
    }
    ~FakeTimerBase() { instance = nullptr; }

    FakeTimerBase(const FakeTimerBase &) = delete;
    FakeTimerBase &operator=(const FakeTimerBase &) = delete;
    FakeTimerBase(FakeTimerBase &&) = delete;
    FakeTimerBase &operator=(FakeTimerBase &&) = delete;

    esp_err_t start(std::chrono::microseconds t) noexcept {
        if (active)
            return ESP_ERR_INVALID_STATE;
        active = true;
        duration = t;
        return ESP_OK;
    }
    esp_err_t restart(std::chrono::microseconds t) noexcept {
        if (!active)
            return ESP_ERR_INVALID_STATE;
        duration = t;
        return ESP_OK;
    }
    esp_err_t stop() noexcept {
        if (!active)
            return ESP_ERR_INVALID_STATE;
        active = false;
        return ESP_OK;
    }
    bool is_active() const noexcept { return active; }

    inline static Self *instance = nullptr;
    timer::callback cb;
    const char *name;
    bool active = false;
    std::chrono::microseconds duration{};
};

class FakePeriodicTimer : public FakeTimerBase<FakePeriodicTimer> {
  public:
    static constexpr timer::kind kind = timer::kind::periodic;
    using FakeTimerBase::FakeTimerBase;

    void fire() {
        if (active)
            cb();
    }
};

class FakeOneShotTimer : public FakeTimerBase<FakeOneShotTimer> {
  public:
    static constexpr timer::kind kind = timer::kind::one_shot;
    using FakeTimerBase::FakeTimerBase;

    MOCK_METHOD(std::optional<std::chrono::microseconds>, remaining, (),
                (const, noexcept));

    void fire() {
        if (!active)
            return;
        active = false;
        cb();
    }
};

static_assert(I_GPIO_IN<FakeGPIO>);
static_assert(I_PeriodicTimer<FakePeriodicTimer>);
static_assert(I_OneShotTimer<FakeOneShotTimer>);

struct FakeButtonTypes {
    using GPIO = FakeGPIO;
    using PeriodicTimer = FakePeriodicTimer;
    using OneShotTimer = FakeOneShotTimer;
};

#endif
