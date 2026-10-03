#ifndef RECT_ITERATOR_TPP
#define RECT_ITERATOR_TPP

#include "rect_it.hpp"

template <typename T>
rect_iterator<T>::rect_iterator(T *topleft, int width, std::ptrdiff_t stride,
                                int row) noexcept
    : topleft{topleft}, width{width}, stride{stride}, row{row} {}

template <typename T>
rect_iterator<T> &rect_iterator<T>::operator++() noexcept {
    if (++col == width) {
        col = 0;
        ++row;
    }
    return *this;
}

template <typename T>
rect_iterator<T> rect_iterator<T>::operator++(int) noexcept {
    auto out = *this;
    ++*this;
    return out;
}

template <typename T>
typename rect_iterator<T>::at_position
rect_iterator<T>::operator*() const noexcept {
    return {topleft[row * stride + col], row, col};
}

template <typename T>
bool rect_iterator<T>::operator==(
    const rect_iterator<T> &other) const noexcept {
    return row == other.row && col == other.col;
}

template <typename T>
std::ranges::subrange<rect_iterator<T>> rect(T *topleft, int width, int height,
                                             std::ptrdiff_t stride) noexcept {
    auto first = rect_iterator<T>{topleft, width, stride};
    auto last = width > 0 && height > 0
                    ? rect_iterator<T>{topleft, width, stride, height}
                    : first;
    return {first, last};
}

#endif
