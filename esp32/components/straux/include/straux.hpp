#ifndef STRAUX_HPP
#define STRAUX_HPP

#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace straux {
template <typename... Args>
std::string strprintf(const char *format, Args... args) {
    int size_s = std::snprintf(nullptr, 0, format, args...) + 1;
    if (size_s <= 0)
        throw std::runtime_error("straux: encoding error.");
    auto size = static_cast<size_t>(size_s);
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format, args...);
    return std::string(buf.get(), buf.get() + size - 1);
}

template <typename T> inline std::span<T> to_span(T *data, size_t size) {
    return {typename std::span<T>::iterator{data}, size};
}

char *strapp(char *dest, const char *source);
char *strcopy(const char *source);

template <typename T> struct NoInitAllocator : std::allocator<T> {
    template <typename U> void construct(U *) {}
};

template <typename T> using cvector = std::vector<T, NoInitAllocator<T>>;
} // namespace straux

#endif
