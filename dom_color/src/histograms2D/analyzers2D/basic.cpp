#include "basic.hpp"
#include "rect_it.hpp"
#include <algorithm>
#include <array>
#include <optional>

namespace hist2D {
namespace analyzers {
namespace {
struct Candidate {
    Dominant dominant;
    uint64_t min_count; // some square of the search is guaranteed to reach it
    uint64_t max_count; // no square of the search can exceed it
};

// Yields the best square of each size 1, 2, ..., 1 << halvings.
class BasicSearchAlgorithm {
  public:
    BasicSearchAlgorithm(const Histogram &histogram,
                         const sumators::Basic &sumator)
        : histogram{histogram}, sumator{sumator},
          clusters{*histogram.get_clusters()} {}

    std::optional<Candidate> get_next() {
        if (histogram.get_count() == 0 || size > 1 << halvings)
            return std::nullopt;
        auto best_window = BasicWindow{get_best(), size};
        size *= 2;
        // The largest squares tile the histogram, so the best of them holds
        // at least their share of the count.
        uint64_t total = histogram.get_count();
        uint64_t largest = 1 << halvings;
        return Candidate{.dominant = to_dominant(best_window),
                         .min_count = total * largest * largest / (sz * sz),
                         .max_count = total};
    }

  private:
    const MyCluster *get_best() const {
        auto width = (int)sz - size + 1;
        auto topleft_corners = rect(&clusters[0][0], width, width, sz);
        auto counts = std::array<uint32_t, sz * sz>{};
        auto counts_end =
            std::transform(topleft_corners.begin(), topleft_corners.end(),
                           counts.begin(), [this](auto cluster) {
                               return sumator.count({&cluster.value, size});
                           });
        auto best =
            std::max_element(counts.begin(), counts_end) - counts.begin();
        return &clusters[best / width][best % width];
    }

    Dominant to_dominant(BasicWindow w) const {
        auto count = sumator.count(w);
        auto sum = sumator.sum(w);
        uint16_t x = count != 0 ? ((uint64_t)sum.x << hshift) / count : 0;
        uint16_t y = count != 0 ? ((uint64_t)sum.y << hshift) / count : 0;
        return {.value = {.x = x, .y = y},
                .bounds = get_bounds(histogram, w),
                .count = count};
    }

    const Histogram &histogram;
    const sumators::Basic &sumator;
    const MyCluster (&clusters)[sz][sz];
    uint8_t size = 1;
};
} // namespace

Basic::Basic(const Histogram &histogram, const sumators::Basic &sumator,
             uint8_t alpha)
    : histogram{histogram}, sumator{sumator}, alpha{alpha} {}

// The first square whose count alpha accepts, if any.
Dominant Basic::get_dominant() const {
    auto search = BasicSearchAlgorithm{histogram, sumator};
    while (auto candidate = search.get_next())
        if (alpha.is_count_enough(candidate->dominant.count,
                                  candidate->min_count, candidate->max_count))
            return candidate->dominant;
    return {};
}
}; // namespace analyzers
}; // namespace hist2D
