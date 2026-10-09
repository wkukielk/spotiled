#include "search_algorithm.hpp"
#include <algorithm>
#include <array>

BasicSearchAlgorithm::BasicSearchAlgorithm(const hist::Histogram &histogram)
    : sumator{histogram, hist::sumators::Basic{histogram.get_clusters()}},
      histogram{histogram}, clusters{histogram.get_clusters()} {}

std::optional<hist::Dominant> BasicSearchAlgorithm::get_next() {
    if (histogram.get_count() == 0 || precission.get_precission() == 0)
        return std::nullopt;
    auto best_window =
        hist::BasicWindow{get_best(), (uint8_t)precission.get_cluster_size()};
    --precission;
    return sumator.window_to_dominant(best_window);
}

const hist::MyCluster *BasicSearchAlgorithm::get_best() const {
    auto current_size = (uint8_t)precission.get_cluster_size();
    auto begin = clusters.begin();
    auto end = clusters.end() - current_size + 1;
    auto counts = std::array<uint32_t, hist::sz>{};
    auto counts_end = std::transform(
        begin, end, counts.begin(), [this, current_size](const auto &c) {
            return sumator.count({&c, current_size});
        });
    auto best = std::max_element(counts.begin(), counts_end);
    return clusters.data() + (best - counts.begin());
}

WrappingSearchAlgorithm::WrappingSearchAlgorithm(
    const hist::Histogram &histogram)
    : sumator{histogram,
              hist::sumators::WrapAround{histogram.get_clusters(),
                                         histogram.get_upper_bound()}},
      histogram{histogram}, clusters{histogram.get_clusters()} {}

std::optional<hist::Dominant> WrappingSearchAlgorithm::get_next() {
    if (histogram.get_count() == 0 || precission.get_precission() == 0)
        return std::nullopt;
    auto best_window =
        hist::BasicWindow{get_best(), (uint8_t)precission.get_cluster_size()};
    --precission;
    return sumator.window_to_dominant(best_window);
}

const hist::MyCluster *WrappingSearchAlgorithm::get_best() const {
    auto current_size = (uint8_t)precission.get_cluster_size();
    auto counts = std::array<uint32_t, hist::sz>{};
    std::transform(clusters.begin(), clusters.end(), counts.begin(),
                   [this, current_size](const auto &c) {
                       return sumator.count({&c, current_size});
                   });
    auto best = std::max_element(counts.begin(), counts.end());
    return clusters.data() + (best - counts.begin());
}

NonLinearSearchAlgorithm::NonLinearSearchAlgorithm(
    const hist::Histogram &histogram)
    : sumator{histogram,
              hist::sumators::NonLinear{histogram.get_clusters(),
                                        histogram.get_upper_bound()}},
      histogram{histogram}, clusters{histogram.get_clusters()} {}

std::optional<hist::Dominant> NonLinearSearchAlgorithm::get_next() {
    if (histogram.get_count() == 0 || depth == hist::halvings)
        return std::nullopt;
    auto best_window = hist::CenterWindow{get_best(), depth};
    ++depth;
    return sumator.window_to_dominant(best_window);
}

const hist::MyCluster *NonLinearSearchAlgorithm::get_best() const {
    auto counts = std::array<uint32_t, hist::sz>{};
    std::transform(
        clusters.begin(), clusters.end(), counts.begin(),
        [this](const auto &c) { return sumator.count({&c, depth}); });
    auto best = std::max_element(counts.begin(), counts.end());
    return clusters.data() + (best - counts.begin());
}
