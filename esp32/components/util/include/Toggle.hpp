#ifndef UTIL_TOGGLE_HPP
#define UTIL_TOGGLE_HPP

#include "util.hpp"
#include <functional>

template <typename T>
concept I_Toggle = requires(T t, const T ct) {
    { ct.is_on() } noexcept -> std::same_as<bool>;
    { t.off() } noexcept -> std::same_as<void>;
    { t.on() } noexcept -> std::same_as<esp_err_t>;
    { t.get() } -> std::same_as<T &>;
};

template <typename T> class Toggle {
  public:
    template <typename... Args> Toggle(Args &&...args);
    esp_err_t on() noexcept;
    void off() noexcept;
    bool is_on() const noexcept;

  private:
    std::move_only_function<T()> factory;
    std::optional<T> obj;
};

static_assert(I_Toggle<Toggle>);

#include "Toggle.tpp"

#endif