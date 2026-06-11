#ifndef INITIALIZER_HPP
#define INITIALIZER_HPP

#include "errors.hpp"
#include <concepts>
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

template <typename T> class Handle {
  public:
    template <typename... Args> Handle(Args... args) {
        if (instance.has_value())
            throw InitError(ESP_FAIL, "Handle already initialized");
        instance.emplace(args...);
    }
    static T &get() { return *instance; }

  private:
    static std::optional<T> instance;
};

template <typename T> std::optional<T> Handle<T>::instance{};

#endif
