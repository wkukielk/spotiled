#ifndef INITIALIZER_HANDLE_HPP
#define INITIALIZER_HANDLE_HPP

#include "errors.hpp"
#include "util.hpp"
#include <format>
#include <optional>

template <typename T>
concept Initializable = requires(T t) {
    { t.deinit() } noexcept -> std::same_as<esp_err_t>;
};

template <Initializable T> class Handle : public util::MoveSafe<Handle<T>> {
  public:
    using Held = T;

    template <typename... Args> Handle(Args &&...args) {
        if (instance.has_value())
            throw InitError(ESP_FAIL,
                            std::format("Handle<{}> already initialized",
                                        util::type_name<T>())
                                .c_str());
        instance.emplace();
        auto err = instance->init(std::forward<Args>(args)...);
        if (err != ESP_OK) {
            instance.reset();
            throw InitError(
                err, std::format("Handle<{}> init failed", util::type_name<T>())
                         .c_str());
        }
    }

    Handle(Handle &&) = default;

    ~Handle() noexcept {
        if (this->last()) {
            instance->deinit();
            instance.reset();
        }
    }

    static T &get() { return *instance; }

  private:
    static std::optional<T> instance;
};

template <Initializable T> std::optional<T> Handle<T>::instance{};

#endif
