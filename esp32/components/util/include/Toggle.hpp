#ifndef UTIL_TOGGLE_HPP
#define UTIL_TOGGLE_HPP

#include "util.hpp"
#include <functional>

template <typename T>
concept I_Toggle = requires(T t, const T ct) {
    { ct.is_on() } noexcept -> std::same_as<bool>;
    { t.off() } noexcept -> std::same_as<void>;
    { t.on() } noexcept -> std::same_as<esp_err_t>;
    { t.get() } -> std::same_as<typename T::Inner &>;
};

template <typename T> class Toggle {
  public:
    using Inner = T;

    template <typename F> Toggle(F &&factory) noexcept;
    esp_err_t on() noexcept;
    void off() noexcept { obj.reset(); }
    bool is_on() const noexcept { return obj.has_value(); }
    T &get() const { return *obj; }

  private:
    std::move_only_function<T()> factory;
    std::optional<T> obj;
};

static_assert(I_Toggle<Toggle<int>>);

#include "Toggle.tpp"

#endif