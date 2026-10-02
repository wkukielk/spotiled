#ifndef DEPTH_ITERATOR_TPP
#define DEPTH_ITERATOR_TPP

#include "depth_it.hpp"

template <std::forward_iterator It>
depth_iterator<It>::depth_iterator(It inner, difference_type offset) noexcept
    : inner{inner}, offset{offset}
{
}

template <std::forward_iterator It>
depth_iterator<It> &depth_iterator<It>::operator++() noexcept
{
    ++inner;
    ++offset;
    return *this;
}

template <std::forward_iterator It>
depth_iterator<It> depth_iterator<It>::operator++(int) noexcept
{
    auto out = *this;
    ++*this;
    return out;
}

template <std::forward_iterator It>
typename depth_iterator<It>::at_distance depth_iterator<It>::operator*() const noexcept
{
    return {*inner, offset < 0 ? -offset : offset};
}

template <std::forward_iterator It>
bool depth_iterator<It>::operator==(const depth_iterator<It> &other) const noexcept
{
    return inner == other.inner && offset == other.offset;
}

#endif
