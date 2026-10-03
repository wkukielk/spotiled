#ifndef SEARCH_ALGORITHM_HPP
#define SEARCH_ALGORITHM_HPP

#include "analyzer.hpp"
#include "histogram.hpp"
#include "sumators/basic.hpp"
#include "sumators/wrap_around.hpp"
#include <optional>

class BasicSearchAlgorithm {
  public:
    BasicSearchAlgorithm(const hist::Histogram &histogram);
    std::optional<hist::Dominant> get_next();

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
    std::optional<hist::Dominant> get_next();

  private:
    const hist::MyCluster *get_best() const;

    const HistogramSumator<hist::sumators::WrapAround> sumator;
    const hist::Histogram &histogram;
    const hist::ClusterArray &clusters;
    hist::Precission precission = hist::Precission{hist::halvings};
};

#endif
