#ifndef SUMATORS_COMMON_HPP
#define SUMATORS_COMMON_HPP

#include "histogram.hpp"
#include <concepts>

namespace hist {
struct BasicWindow {
    const hist::MyCluster *first;
    uint8_t size;
};

struct CenterWindow {
    const hist::MyCluster *center;
    uint8_t depth;
};

Bounds get_bounds(const Histogram &histogram, BasicWindow w);
Bounds get_bounds(const Histogram &histogram, CenterWindow w);

template <typename S>
concept Sumator =
    requires(S s, typename S::window_t w, hist::Histogram &hgram) {
        typename S::window_t;
        s.count(w);
        s.sum(w);
        { get_bounds(hgram, w) } -> std::same_as<Bounds>;
    };
} // namespace hist

#endif
