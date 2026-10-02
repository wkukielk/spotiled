#ifndef LAP_ITERATOR_TPP
#define LAP_ITERATOR_TPP

#include "lap_it.hpp"

template <WrapContainer C>
lap_iterator<C>::lap_iterator(wraparound_iterator<C> it) noexcept
    : it{it}
{
}

template <WrapContainer C>
lap_iterator<C> &lap_iterator<C>::operator++() noexcept
{
    return *this += 1;
}

template <WrapContainer C>
lap_iterator<C> lap_iterator<C>::operator++(int) noexcept
{
    auto out = *this;
    ++*this;
    return out;
}

template <WrapContainer C>
lap_iterator<C> &lap_iterator<C>::operator--() noexcept
{
    return *this -= 1;
}

template <WrapContainer C>
lap_iterator<C> lap_iterator<C>::operator--(int) noexcept
{
    auto out = *this;
    --*this;
    return out;
}

template <WrapContainer C>
lap_iterator<C> &lap_iterator<C>::operator+=(difference_type n) noexcept
{
    const auto &container = it.get_container();
    auto size = (difference_type)std::ranges::size(container);
    auto position = (it.base() - std::ranges::begin(container)) + n;
    // floor(position / size)
    lap += position >= 0 ? position / size : -((size - 1 - position) / size);
    if (n >= 0)
        it += (int)n;
    else
        it -= (int)-n;
    return *this;
}

template <WrapContainer C>
lap_iterator<C> &lap_iterator<C>::operator-=(difference_type n) noexcept
{
    return *this += -n;
}

template <WrapContainer C>
typename lap_iterator<C>::lapped lap_iterator<C>::operator*() const noexcept
{
    return {*it, lap};
}

template <WrapContainer C>
bool lap_iterator<C>::operator==(const lap_iterator<C> &other) const noexcept
{
    return it == other.it && lap == other.lap;
}

template <typename C>
lap_iterator<C> operator+(lap_iterator<C> it, typename lap_iterator<C>::difference_type n) noexcept
{
    return it += n;
}

template <typename C>
lap_iterator<C> operator-(lap_iterator<C> it, typename lap_iterator<C>::difference_type n) noexcept
{
    return it -= n;
}

#endif
