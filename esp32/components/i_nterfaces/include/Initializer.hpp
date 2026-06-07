#ifndef INITIALIZER_HPP
#define INITIALIZER_HPP

#include "esp_log.h"
#include <concepts>
#include <format>
#include <optional>

template <typename T>
concept Initializable = requires {
    { T::create() };
    { T::cleanup() };
};

template <Initializable T> class Initializer {
  public:
    Initializer() { T::create(); }
    ~Initializer() noexcept { T::cleanup(); }
};

#endif
