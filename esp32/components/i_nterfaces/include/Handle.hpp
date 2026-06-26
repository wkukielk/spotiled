#ifndef INITIALIZER_HANDLE_HPP
#define INITIALIZER_HANDLE_HPP

#include "errors.hpp"
#include <concepts>
#include <optional>

template <typename T> class Handle {
  public:
    template <typename... Args> Handle(Args... args) {
        if (instance.has_value())
            throw InitError(ESP_FAIL, "Handle already initialized");
        instance.emplace(args...);
    }
    Handle(Handle&&) = default;
    ~Handle() noexcept { instance.reset(); }
    static T &get() { return *instance; }

  private:
    static std::optional<T> instance;
};

template <typename T> std::optional<T> Handle<T>::instance{};

#endif
