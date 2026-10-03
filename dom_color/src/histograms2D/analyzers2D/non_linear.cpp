#include "non_linear.hpp"
#include "rect_it.hpp"
#include <algorithm>
#include <array>
#include <functional>
#include <numeric>
#include <optional>

namespace hist2D {
namespace analyzers {
namespace {
struct Candidate {
    Dominant dominant;
    uint64_t min_count; // some neighbourhood of the search is guaranteed to
                        // reach it
    uint64_t max_count; // no neighbourhood of the search can exceed it
};

// How much a corner cluster adds, in total, to the neighbourhoods of `depth`
// around all centres. Corner clusters lie in the fewest neighbourhoods, so
// every cluster adds at least this much. The weights are symmetric, so it is
// the (depth + 1) x (depth + 1) block of weights starting at the centre.
uint64_t corner_coverage(uint8_t depth) {
    auto block = rect(&sumators::NonLinear::weights[2][2], depth + 1,
                      depth + 1, 5);
    return std::transform_reduce(block.begin(), block.end(), uint64_t{0},
                                 std::plus{},
                                 [](auto weight) { return weight.value; });
}

// Yields the best neighbourhood of each depth 0, 1, ..., halvings - 1.
class NonLinearSearchAlgorithm {
  public:
    NonLinearSearchAlgorithm(const Histogram &histogram,
                             const sumators::NonLinear &sumator)
        : histogram{histogram}, sumator{sumator},
          clusters{*histogram.get_clusters()} {}

    std::optional<Candidate> get_next() {
        if (histogram.get_count() == 0 || depth == halvings)
            return std::nullopt;
        auto best_window = CenterWindow{get_best(), depth};
        ++depth;
        // The deepest neighbourhoods, summed over all centres, count every
        // cluster at least corner_coverage times, so the best of them reaches
        // at least the average. A neighbourhood scores most when everything
        // sits in its centre.
        uint64_t total = histogram.get_count();
        return Candidate{
            .dominant = to_dominant(best_window),
            .min_count = corner_coverage(halvings - 1) * total / (sz * sz),
            .max_count = sumators::NonLinear::weights[2][2] * total};
    }

  private:
    const MyCluster *get_best() const {
        auto centers = rect(&clusters[0][0], sz, sz, sz);
        auto counts = std::array<uint64_t, sz * sz>{};
        std::transform(centers.begin(), centers.end(), counts.begin(),
                       [this](auto cluster) {
                           return sumator.count({&cluster.value, depth});
                       });
        auto best = std::max_element(counts.begin(), counts.end()) - counts.begin();
        return &clusters[best / sz][best % sz];
    }

    Dominant to_dominant(CenterWindow w) const {
        auto count = sumator.count(w);
        auto sum = sumator.sum(w);
        uint16_t x = count != 0 ? (sum.x << hshift) / count : 0;
        uint16_t y = count != 0 ? (sum.y << hshift) / count : 0;
        return {.value = {.x = x, .y = y},
                .bounds = get_bounds(histogram, w),
                .count = count};
    }

    const Histogram &histogram;
    const sumators::NonLinear &sumator;
    const MyCluster (&clusters)[sz][sz];
    uint8_t depth = 0;
};
} // namespace

NonLinear::NonLinear(const Histogram &histogram,
                     const sumators::NonLinear &sumator, uint8_t alpha)
    : histogram{histogram}, sumator{sumator}, alpha{alpha} {}

// The first neighbourhood whose count alpha accepts, if any.
Dominant NonLinear::get_dominant() const {
    auto search = NonLinearSearchAlgorithm{histogram, sumator};
    while (auto candidate = search.get_next())
        if (alpha.is_count_enough(candidate->dominant.count,
                                  candidate->min_count, candidate->max_count))
            return candidate->dominant;
    return {};
}
}; // namespace analyzers
}; // namespace hist2D
