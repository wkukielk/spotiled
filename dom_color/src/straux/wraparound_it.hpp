#ifndef WRAP_AROUND_ITERATOR
#define WRAP_AROUND_ITERATOR

#include <iterator>
#include <ranges>
#include <stdexcept>

template <class C>
concept WrapContainer = std::ranges::random_access_range<const C> &&
                        std::ranges::sized_range<const C>;

template <WrapContainer C>
class wraparound_iterator
{
public:
    using iterator = std::ranges::iterator_t<const C>;

    wraparound_iterator(const C &container, iterator pointer);

    iterator base() const noexcept { return pointer; }

    wraparound_iterator &operator++() noexcept;
    wraparound_iterator operator++(int) noexcept;
    wraparound_iterator &operator--() noexcept;
    wraparound_iterator operator--(int) noexcept;
    wraparound_iterator &operator+=(int n) noexcept;
    wraparound_iterator &operator-=(int n) noexcept;

    std::ranges::range_reference_t<const C> operator*() const noexcept;
    iterator operator->() const noexcept;

    bool operator==(const wraparound_iterator<C> &other) const noexcept;
    bool operator!=(const wraparound_iterator<C> &other) const noexcept;

private:
    const C &container;
    iterator pointer;
};

template <typename C>
wraparound_iterator<C> operator+(wraparound_iterator<C> it, int n) noexcept;

template <typename C>
wraparound_iterator<C> operator-(wraparound_iterator<C> it, int n) noexcept;

#include "wraparound_it.tpp"

#endif
