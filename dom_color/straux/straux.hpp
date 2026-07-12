#ifndef STRAUX_HPP
#define STRAUX_HPP

#include <memory>
#include <span>
#include <stdexcept>
#include <string>

namespace straux
{
    template <typename T>
    inline std::span<T> to_span(T *data, size_t size);

    template <typename T>
    struct buf
    {
    public:
        std::unique_ptr<T[]> data;
        size_t size;
        std::span<T> get_span() const noexcept;
        std::span<const T> get_cspan() const noexcept;
        T *end() noexcept;
        const T *cend() const noexcept;
        bool operator==(const buf<T> &other) const noexcept;
    };

    template <typename T>
    inline std::span<T> to_span(T *data, size_t size)
    {
        return {typename std::span<T>::iterator{data}, size};
    }

    template <typename T>
    std::span<T> buf<T>::get_span() const noexcept
    {
        return to_span(data.get(), size);
    }

    template <typename T>
    std::span<const T> buf<T>::get_cspan() const noexcept
    {
        return to_span(data.get(), size);
    }

    template <typename T>
    T *buf<T>::end() noexcept { return data.get() + size; }

    template <typename T>
    const T *buf<T>::cend() const noexcept
    {
        return data.get() + size;
    }

    template <typename T>
    bool buf<T>::operator==(const buf<T> &other) const noexcept
    {
        return data == other.data && size == other.size;
    }

    char *strapp(char *dest, const char *source);
    char *strcopy(const char *source);
} // namespace straux

#endif
