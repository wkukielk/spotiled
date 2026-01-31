#ifndef SAFE_UNIQUE_PTR_HPP
#define SAFE_UNIQUE_PTR_HPP

#include "util.hpp"
#include <memory>
#include <mutex>

template <typename T> class safe_unique_ptr {
  public:
    safe_unique_ptr(T *ptr = nullptr);
    safe_unique_ptr(std::unique_ptr<T> &&ptr);
    safe_unique_ptr<T> &operator=(std::unique_ptr<T> &&ptr);
    T *operator->();
    T &operator*();
    void reset(T *ptr = nullptr) noexcept;
    T *get();
    const T *get() const;
    operator bool() const;

  private:
    std::unique_ptr<T> ptr;
    std::mutex m;
};

template <typename T> safe_unique_ptr<T>::safe_unique_ptr(T *ptr) : ptr{ptr} {}

template <typename T>
safe_unique_ptr<T>::safe_unique_ptr(std::unique_ptr<T> &&ptr)
    : ptr{std::move(ptr)} {}

template <typename T>
safe_unique_ptr<T> &safe_unique_ptr<T>::operator=(std::unique_ptr<T> &&ptr) {
    this->ptr = std::move(ptr);
    return *this;
}
template <typename T> T *safe_unique_ptr<T>::operator->() {
    UTIL_CHECK(ptr.get() == nullptr, "no pointer");
    return ptr.get();
}
template <typename T> T &safe_unique_ptr<T>::operator*() {
    UTIL_CHECK(ptr.get() == nullptr, "no pointer");
    return *ptr;
}
template <typename T> void safe_unique_ptr<T>::reset(T *ptr) noexcept {
    this->ptr.reset(ptr);
}
template <typename T> T *safe_unique_ptr<T>::get() {
    UTIL_CHECK(ptr.get() == nullptr, "no pointer");
    return ptr.get();
}
template <typename T> const T *safe_unique_ptr<T>::get() const {
    UTIL_CHECK(ptr.get() == nullptr, "no pointer");
    return ptr.get();
}

template <typename T> safe_unique_ptr<T>::operator bool() const {
    return ptr.get();
}

#endif