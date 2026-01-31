#ifndef ATOMIC_UNIQUE_PTR_HPP
#define ATOMIC_UNIQUE_PTR_HPP

#include <memory>
#include <mutex>

template <typename T> class atomic_unique_ptr {
  public:
    atomic_unique_ptr(T *ptr = nullptr);
    atomic_unique_ptr(std::unique_ptr<T> &&ptr);
    atomic_unique_ptr<T> &operator=(std::unique_ptr<T> &&ptr);
    T *operator->() noexcept;
    T &operator*() noexcept;
    void reset(T *ptr = nullptr) noexcept;
    T *get() noexcept;

  private:
    std::unique_ptr<T> ptr;
    std::mutex m;
};

template <typename T>
atomic_unique_ptr<T>::atomic_unique_ptr(T *ptr) : ptr{ptr} {}

template <typename T>
atomic_unique_ptr<T>::atomic_unique_ptr(std::unique_ptr<T> &&ptr)
    : ptr{std::move(ptr)} {}

template <typename T>
atomic_unique_ptr<T> &
atomic_unique_ptr<T>::operator=(std::unique_ptr<T> &&ptr) {
    std::lock_guard<std::mutex> lock{m};
    this->ptr = std::move(ptr);
    return *this;
}
template <typename T> T *atomic_unique_ptr<T>::operator->() noexcept {
    std::lock_guard<std::mutex> lock{m};
    return ptr.get();
}
template <typename T> T &atomic_unique_ptr<T>::operator*() noexcept {
    std::lock_guard<std::mutex> lock{m};
    return *ptr;
}
template <typename T> void atomic_unique_ptr<T>::reset(T *ptr) noexcept {
    std::lock_guard<std::mutex> lock{m};
    this->ptr.reset(ptr);
}
template <typename T> T *atomic_unique_ptr<T>::get() noexcept {
    std::lock_guard<std::mutex> lock{m};
    return ptr.get();
}

#endif
