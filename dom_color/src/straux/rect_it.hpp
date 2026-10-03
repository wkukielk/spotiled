#ifndef RECT_ITERATOR
#define RECT_ITERATOR

#include <cstddef>
#include <iterator>
#include <ranges>

// Walks a rectangle of a row-major 2D array row by row, reporting each
// element's row and column within the rectangle.
template <typename T>
class rect_iterator
{
public:
    struct at_position
    {
        T &value;
        int row;
        int col;
    };

    using iterator_concept = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = at_position;

    rect_iterator() = default;
    // `stride` is the length of a row of the whole array. Starts at the first
    // element of `row`; row == height is the end of the rectangle.
    rect_iterator(T *topleft, int width, std::ptrdiff_t stride,
                  int row = 0) noexcept;

    rect_iterator &operator++() noexcept;
    rect_iterator operator++(int) noexcept;

    at_position operator*() const noexcept;

    bool operator==(const rect_iterator<T> &other) const noexcept;

private:
    T *topleft = nullptr;
    int width = 0;
    std::ptrdiff_t stride = 0;
    int row = 0;
    int col = 0;
};

// The width x height rectangle starting at `topleft`.
template <typename T>
std::ranges::subrange<rect_iterator<T>> rect(T *topleft, int width, int height,
                                             std::ptrdiff_t stride) noexcept;

#include "rect_it.tpp"

#endif
