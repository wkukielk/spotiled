#ifndef LAP_ITERATOR
#define LAP_ITERATOR

#include "wraparound_it.hpp"
#include <iterator>

// Decorates wraparound_iterator with the number of times it has passed the end
// of the container since it was constructed (negative when moved backwards).
template <WrapContainer C>
class lap_iterator
{
public:
    struct lapped
    {
        const std::ranges::range_value_t<const C> &value;
        int lap;
    };

    using iterator_concept = std::forward_iterator_tag;
    using difference_type = typename wraparound_iterator<C>::difference_type;
    using value_type = lapped;

    lap_iterator() = default;
    explicit lap_iterator(wraparound_iterator<C> it) noexcept;

    const wraparound_iterator<C> &base() const noexcept { return it; }
    int get_lap() const noexcept { return lap; }

    lap_iterator &operator++() noexcept;
    lap_iterator operator++(int) noexcept;
    lap_iterator &operator--() noexcept;
    lap_iterator operator--(int) noexcept;
    lap_iterator &operator+=(difference_type n) noexcept;
    lap_iterator &operator-=(difference_type n) noexcept;

    lapped operator*() const noexcept;

    bool operator==(const lap_iterator<C> &other) const noexcept;

private:
    wraparound_iterator<C> it;
    int lap = 0;
};

template <typename C>
lap_iterator<C> operator+(lap_iterator<C> it, typename lap_iterator<C>::difference_type n) noexcept;

template <typename C>
lap_iterator<C> operator-(lap_iterator<C> it, typename lap_iterator<C>::difference_type n) noexcept;

#include "lap_it.tpp"

#endif
