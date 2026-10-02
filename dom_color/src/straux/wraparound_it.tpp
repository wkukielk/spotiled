#ifndef WRAP_AROUND_ITERATOR_TPP
#define WRAP_AROUND_ITERATOR_TPP

#include "wraparound_it.hpp"

template <WrapContainer C>
wraparound_iterator<C>::wraparound_iterator(const C &container, iterator pointer)
    : container{&container}, pointer{pointer}
{
    if (pointer < container.begin() || container.end() <= pointer)
        throw std::runtime_error{"pointer out of range"};
}

template <WrapContainer C>
wraparound_iterator<C> &wraparound_iterator<C>::operator++() noexcept
{
    if (++pointer == container->end())
        pointer = container->begin();
    return *this;
}

template <WrapContainer C>
wraparound_iterator<C> wraparound_iterator<C>::operator++(int) noexcept
{
    auto out = *this;
    ++*this;
    return out;
}

template <WrapContainer C>
wraparound_iterator<C> &wraparound_iterator<C>::operator--() noexcept
{
    if (pointer-- == container->begin())
        pointer = container->end() - 1;
    return *this;
}

template <WrapContainer C>
wraparound_iterator<C> wraparound_iterator<C>::operator--(int) noexcept
{
    auto out = *this;
    --*this;
    return out;
}

template <WrapContainer C>
wraparound_iterator<C> &wraparound_iterator<C>::operator+=(int n) noexcept
{
    pointer += n;
    while (pointer >= container->end())
        pointer -= container->size();
    return *this;
}

template <WrapContainer C>
wraparound_iterator<C> &wraparound_iterator<C>::operator-=(int n) noexcept
{
    pointer -= n;
    while (pointer < container->begin())
        pointer += container->size();
    return *this;
}

template <WrapContainer C>
std::ranges::range_reference_t<const C> wraparound_iterator<C>::operator*() const noexcept
{
    return *pointer;
}

template <WrapContainer C>
typename wraparound_iterator<C>::iterator wraparound_iterator<C>::operator->() const noexcept
{
    return pointer;
}

template <WrapContainer C>
bool wraparound_iterator<C>::operator==(const wraparound_iterator<C> &other) const noexcept
{
    return pointer == other.pointer;
}

template <WrapContainer C>
bool wraparound_iterator<C>::operator!=(const wraparound_iterator<C> &other) const noexcept
{
    return !(*this == other);
}

template <typename C>
wraparound_iterator<C> operator+(wraparound_iterator<C> it, int n) noexcept
{
    return it += n;
}

template <typename C>
wraparound_iterator<C> operator-(wraparound_iterator<C> it, int n) noexcept
{
    return it -= n;
}

#endif
