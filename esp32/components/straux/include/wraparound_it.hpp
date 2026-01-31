#ifndef WRAP_AROUND_ITERATOR
#define WRAP_AROUND_ITERATOR

#include <stdexcept>

template <class C>
concept WrapContainer = requires {
    typename C::const_iterator;
    requires std::random_access_iterator<typename C::const_iterator>;
};

template <WrapContainer C> class wraparound_iterator {
  public:
    wraparound_iterator(const C &container, typename C::const_iterator pointer)
        : container{container}, pointer{pointer} {
        if (pointer < container.begin() || container.end() <= pointer)
            throw std::runtime_error{"pointer out of range"};
    };
    typename C::const_iterator base() const { return pointer; }
    wraparound_iterator &operator++() {
        if (++pointer == container.end())
            pointer = container.begin();
        return *this;
    }
    wraparound_iterator operator++(int) {
        auto out = *this;
        ++*this;
        return out;
    }
    wraparound_iterator &operator--() {
        if (pointer-- == container.begin())
            pointer = container.end() - 1;
        return *this;
    }
    wraparound_iterator operator--(int) {
        auto out = *this;
        --*this;
        return out;
    }
    wraparound_iterator &operator+=(int n) {
        pointer += n;
        while (pointer >= container.end())
            pointer -= container.size();
        return *this;
    }
    wraparound_iterator &operator-=(int n) {
        pointer -= n;
        while (pointer < container.begin())
            pointer += container.size();
        return *this;
    }
    typename C::value_type &operator*() { return *pointer; }
    typename C::const_iterator operator->() { return pointer; }
    bool operator==(const wraparound_iterator &other) {
        return pointer == other.pointer;
    }
    bool operator!=(const wraparound_iterator &other) {
        return !(*this == other);
    }

  private:
    const C &container;
    typename C::const_iterator pointer = nullptr;
};

template <typename C>
wraparound_iterator<C> operator+(wraparound_iterator<C> it, int n) {
    return it += n;
}

template <typename C>
wraparound_iterator<C> operator-(wraparound_iterator<C> it, int n) {
    return it -= n;
}

#endif
