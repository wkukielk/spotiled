#include "basic.hpp"
#include "rect_it.hpp"
#include <functional>
#include <numeric>


namespace hist2D {
    namespace sumators {
        namespace {
            // Sum of value_of(cluster) over the square_size x square_size
            // square whose top-left cluster is `topleft`.
            template <typename ValueOf>
            uint32_t square_sum(const MyCluster* topleft, uint8_t square_size,
                    ValueOf value_of) {
                auto square = rect(topleft, square_size, square_size, sz);
                return std::transform_reduce(square.begin(), square.end(),
                    uint32_t{0}, std::plus{},
                    [=](auto cluster) { return value_of(cluster.value); });
            }
        }

        Basic::Basic(const MyCluster (*clusters)[sz][sz]) :
            begin{(MyCluster*)clusters}, end{(MyCluster*)(clusters + 1)} {}

        uint32_t Basic::count(BasicWindow w) const {
            return square_sum(w.topleft, w.size,
                [](const MyCluster& cluster) { return (uint32_t)cluster.count; });
        }

        Sum Basic::sum(BasicWindow w) const {
            return {
                .x = square_sum(w.topleft, w.size,
                    [](const MyCluster& cluster) { return cluster.sum_x; }),
                .y = square_sum(w.topleft, w.size,
                    [](const MyCluster& cluster) { return cluster.sum_y; })
            };
        }
    };
};
