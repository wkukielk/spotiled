#ifndef WRAP_AROUND_ITERATOR
#define WRAP_AROUND_ITERATOR

#include <iterator>
#include <stdexcept>

template <class C>
concept WrapContainer = requires {
    typename C::const_iterator;
    requires std::random_access_iterator<typename C::const_iterator>;
};

template <WrapContainer C>
class wraparound_iterator
{
public:
    wraparound_iterator(const C &container, typename C::const_iterator pointer);

    typename C::const_iterator base() const noexcept { return pointer; }

    wraparound_iterator &operator++() noexcept;
    wraparound_iterator operator++(int) noexcept;
    wraparound_iterator &operator--() noexcept;
    wraparound_iterator operator--(int) noexcept;
    wraparound_iterator &operator+=(int n) noexcept;
    wraparound_iterator &operator-=(int n) noexcept;

    const typename C::value_type &operator*() const noexcept;
    typename C::const_iterator operator->() const noexcept;

    bool operator==(const wraparound_iterator<C> &other) const noexcept;
    bool operator!=(const wraparound_iterator<C> &other) const noexcept;

private:
    const C &container;
    typename C::const_iterator pointer = nullptr;
};

template <typename C>
wraparound_iterator<C> operator+(wraparound_iterator<C> it, int n) noexcept;

template <typename C>
wraparound_iterator<C> operator-(wraparound_iterator<C> it, int n) noexcept;

#include "wraparound_it.tpp"

#endif
