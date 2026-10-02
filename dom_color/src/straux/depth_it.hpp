#ifndef DEPTH_ITERATOR
#define DEPTH_ITERATOR

#include <iterator>

// Decorates another iterator with its distance from a centre element.
template <std::forward_iterator It>
class depth_iterator
{
public:
    struct at_distance
    {
        std::iter_reference_t<It> value;
        std::iter_difference_t<It> distance;
    };

    using iterator_concept = std::forward_iterator_tag;
    using difference_type = std::iter_difference_t<It>;
    using value_type = at_distance;

    depth_iterator() = default;
    // `offset` is the position of `inner` relative to the centre (negative before it).
    depth_iterator(It inner, difference_type offset) noexcept;

    const It &base() const noexcept { return inner; }

    depth_iterator &operator++() noexcept;
    depth_iterator operator++(int) noexcept;

    at_distance operator*() const noexcept;

    bool operator==(const depth_iterator<It> &other) const noexcept;

private:
    It inner{};
    difference_type offset = 0;
};

#include "depth_it.tpp"

#endif
