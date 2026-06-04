#ifndef UTIL_TOGGLE_HPP
#include "Toggle.hpp"
#endif

template <typename T>
template <typename... Args>
Toggle<T>::Toggle(Args &&...args)
    : factory{[=, ... args = std::forward<Args>(args)]() -> T {
          return T(args...);
      }} {}

template <typename T> esp_err_t Toggle<T>::on() noexcept {
    UTIL_RETURN(obj.has_value());
    obj = factory();
    return ESP_OK;
}

template <typename T> void Toggle<T>::off() noexcept { obj.reset() }

template <typename T> bool Toggle<T>::is_on() const noexcept {
    return obj.has_value();
}
