#ifndef UTIL_TOGGLE_HPP
#include "Toggle.hpp"
#endif

template <typename T>
template <typename F>
Toggle<T>::Toggle(F &&factory) noexcept : factory(std::forward<F>(factory)) {}

template <typename T> esp_err_t Toggle<T>::on() noexcept {
    UTIL_RETURN(obj.has_value());
    obj.emplace(factory());
    return ESP_OK;
}
