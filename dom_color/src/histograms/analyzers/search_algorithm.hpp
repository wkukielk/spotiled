#ifndef SEARCH_ALGORITHM_HPP
#define SEARCH_ALGORITHM_HPP

#include "histogram.hpp"
#include "histogram_sumator.hpp"
#include "sumators/basic.hpp"
#include "sumators/non_linear.hpp"
#include "sumators/wrap_around.hpp"
#include <concepts>
#include <optional>

// A dominant window found by a search, with the range its count is judged in.
struct Candidate {
    hist::Dominant dominant;
    uint64_t min_count; // some window of the search is guaranteed to reach it
    uint64_t max_count; // no window of the search can exceed it
};

template <typename S>
concept SearchAlgorithm = std::constructible_from<S, const hist::Histogram &> &&
                          requires(S s) {
                              {
                                  s.get_next()
                              } -> std::same_as<std::optional<Candidate>>;
                          };

class BasicSearchAlgorithm {
  public:
    BasicSearchAlgorithm(const hist::Histogram &histogram);
    std::optional<Candidate> get_next();

  private:
    const hist::MyCluster *get_best() const;

    const HistogramSumator<hist::sumators::Basic> sumator;
    const hist::Histogram &histogram;
    const hist::ClusterArray &clusters;
    hist::Precission precission = hist::Precission{hist::halvings};
};

class WrappingSearchAlgorithm {
  public:
    WrappingSearchAlgorithm(const hist::Histogram &histogram);
    std::optional<Candidate> get_next();

  private:
    const hist::MyCluster *get_best() const;

    const HistogramSumator<hist::sumators::WrapAround> sumator;
    const hist::Histogram &histogram;
    const hist::ClusterArray &clusters;
    hist::Precission precission = hist::Precission{hist::halvings};
};

class NonLinearSearchAlgorithm {
  public:
    NonLinearSearchAlgorithm(const hist::Histogram &histogram);
    std::optional<Candidate> get_next();

  private:
    const hist::MyCluster *get_best() const;

    const HistogramSumator<hist::sumators::NonLinear> sumator;
    const hist::Histogram &histogram;
    const hist::ClusterArray &clusters;
    uint8_t depth = 0;
};

static_assert(SearchAlgorithm<BasicSearchAlgorithm>);
static_assert(SearchAlgorithm<WrappingSearchAlgorithm>);
static_assert(SearchAlgorithm<NonLinearSearchAlgorithm>);

#endif
